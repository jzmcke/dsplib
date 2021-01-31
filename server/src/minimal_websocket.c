// Simple LibWebSockets test client
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "libwebsockets.h"
#include "minimal_websocket.h"
#include <pthread.h>

#define MAX_TRANSMIT_BYTES 4096

typedef struct transmit_data
{
	unsigned char *p_data;
	size_t n_bytes;
} transmit_data;


struct minimal_websocket_s
{
	int port;
	char *addr;
	transmit_data *p_transmit_data;
};

static int bExit;
static int bDenyDeflate = 1;

static int callback_service(struct lws* wsi, enum lws_callback_reasons reason, void *user, void* in, size_t len);

void *
client_run_and_listen(void *p_thread_context);

void *
client_increment_and_transmit(void *p_thread_context);

typedef struct context_lock
{
	struct lws_context *p_context;
	struct lws *p_lws;
	int b_locked;
} context_lock;

// Escape the loop when a SIGINT signal is received
static void onSigInt(int sig)
{
	bExit = 1;
}

// The registered protocols
static struct lws_protocols protocols[] = {
	{
		"transmit-receive-service", // Protocol name
		callback_service,   // Protocol callback
		sizeof(transmit_data), // This is where we'll store the data to be transmitted.
		512,			 // Receive buffer size (can be left empty)

	},
	{ NULL, NULL, 0 } // Always needed at the end
};

// The extensions LWS supports, without them some requests may not be able to work
static const struct lws_extension extensions[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_max_window_bits"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL } // Always needed at the end
};

// List to identify the indices of the protocols by name
enum protocolList {
	PROTOCOL_TEST,

	PROTOCOL_LIST_COUNT // Needed
};

// Callback for the test protocol
static int
callback_service(struct lws* wsi, enum lws_callback_reasons reason, void *user, void* in, size_t len)
{
	// The message we send back to the echo server
	const char msg[MAX_TRANSMIT_BYTES] = "20";

	// The buffer holding the data to send
	// NOTICE: data which is sent always needs to have a certain amount of memory (LWS_PRE) preserved for headers
	unsigned char buf[LWS_PRE + MAX_TRANSMIT_BYTES];
	unsigned char counter_buf[LWS_PRE + MAX_TRANSMIT_BYTES];
	transmit_data *p_transmit_data = (transmit_data*)lws_context_user(lws_get_context(user));

	// Allocating the memory for the buffer, and copying the message to it
	memset(&buf[LWS_PRE], 0, MAX_TRANSMIT_BYTES);
	strncpy((char*)buf + LWS_PRE, msg, MAX_TRANSMIT_BYTES);

	// For which reason was this callback called?
	switch (reason)
	{
		// The connection closed
	case LWS_CALLBACK_CLOSED:
		printf("[Test Protocol] Connection closed.\n");
		break;

		// Our client received something
	case LWS_CALLBACK_CLIENT_RECEIVE:
	{
		printf("[Test Protocol] Received data: \"%s\"\n", (char*)in);

		bExit = 0;

		//return -1; // Returning -1 causes the client to disconnect from the server
	}
		break;

		// Here the server tries to confirm if a certain extension is supported by the server
	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		if (strcmp((char*)in, "deflate-stream") == 0)
		{
			if (bDenyDeflate)
			{
				printf("[Test Protocol] Denied deflate-stream extension\n");
				return 1;
			}
		}
		break;

		// The connection was successfully established
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		printf("[Test Protocol] Connection to server established.\n");

		printf("[Test Protocol] Writing \"%s\" to server.\n", msg);

		// Write the buffer from the LWS_PRE index + 128 (the buffer size)
		lws_write(wsi, &buf[LWS_PRE], MAX_TRANSMIT_BYTES, LWS_WRITE_TEXT);

		break;

		// The server notifies us that we can write data
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		printf("[Test Protocol] The client is able to write.\n");
		// Allocating the memory for the buffer, and copying the message to it
		memset(&counter_buf[LWS_PRE], 0, 4096);

		assert(p_transmit_data->n_bytes < 4096);

		memcpy(&counter_buf[LWS_PRE], p_transmit_data->p_data, p_transmit_data->n_bytes);

		lws_write(wsi, &counter_buf[LWS_PRE], LWS_PRE+p_transmit_data->n_bytes, LWS_WRITE_TEXT);
		
		break;

		// There was an error connecting to the server
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		printf("[Test Protocol] There was a connection error: %s\n", in ? (char*)in : "(no error information)");
		break;

	default:
		break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

// Main application entry
int
minimal_websocket_init(minimal_websocket **pp_self, minimal_websocket_cfg *p_cfg)
{
	lws_set_log_level(LLL_ERR | LLL_WARN, lwsl_emit_syslog); // We don't need to see the notice messages

	signal(SIGINT, onSigInt); // Register the SIGINT handler

	// Connection info
	char inputURL[300] = "ws://";
	int inputPort = p_cfg->port;

	strcat(inputURL, p_cfg->addr);

	struct lws_context_creation_info ctxCreationInfo; // Context creation info
	struct lws_client_connect_info clientConnectInfo; // Client creation info
	struct lws_context *ctx; // The context to use
	context_lock ctx_locker;
	transmit_data *p_transmit_data;

	pthread_t client_listen_thread_id;
	pthread_t client_main_thread_id;

	struct lws *wsiTest; // WebSocket interface
	const char *urlProtocol, *urlTempPath; // the protocol of the URL, and a temporary pointer to the path
	char urlPath[300]; // The final path string

	p_transmit_data = (transmit_data*)malloc(sizeof(transmit_data));

	// Set both information to empty and allocate it's memory
	memset(&ctxCreationInfo, 0, sizeof(ctxCreationInfo));
	memset(&clientConnectInfo, 0, sizeof(clientConnectInfo));

	clientConnectInfo.port = inputPort; // Set the client info's port to the input port
	
	// Parse the input url (e.g. wss://echo.websocket.org:1234/test)
	//   the protocol (wss)
	//   the address (echo.websocket.org)
	//   the port (1234)
	//   the path (/test)
	if (lws_parse_uri(inputURL, &urlProtocol, &clientConnectInfo.address, &clientConnectInfo.port, &urlTempPath))
	{
		printf("Couldn't parse URL\n");
	}
	clientConnectInfo.port = inputPort;
	// Fix up the urlPath by adding a / at the beginning, copy the temp path, and add a \0 at the end
	urlPath[0] = '/';
	strncpy(urlPath + 1, urlTempPath, sizeof(urlPath) - 2);
	urlPath[sizeof(urlPath) - 1] = '\0';

	clientConnectInfo.path = urlPath; // Set the info's path to the fixed up url path

	// Set up the context creation info
	ctxCreationInfo.port = CONTEXT_PORT_NO_LISTEN; // We don't want this client to listen
	ctxCreationInfo.protocols = protocols; // Use our protocol list
	ctxCreationInfo.gid = -1; // Set the gid and uid to -1, isn't used much
	ctxCreationInfo.uid = -1;
	ctxCreationInfo.user = p_transmit_data;
	ctxCreationInfo.extensions = extensions; // Use our extensions list

	// Create the context with the info
	ctx = lws_create_context(&ctxCreationInfo);
	if (ctx == NULL)
	{
		printf("Error creating context\n");
		return 1;
	}

	// Set up the client creation info
	clientConnectInfo.context = ctx; // Use our created context
	clientConnectInfo.ssl_connection = 0; // Don't use SSL for this test
	clientConnectInfo.host = clientConnectInfo.address; // Set the connections host to the address
	clientConnectInfo.origin = clientConnectInfo.address; // Set the conntections origin to the address
	clientConnectInfo.ietf_version_or_minus_one = -1; // IETF version is -1 (the latest one)
	clientConnectInfo.protocol = protocols[PROTOCOL_TEST].name; // We use our test protocol
	clientConnectInfo.pwsi = &wsiTest; // The created client should be fed inside the wsi_test variable

	printf("Connecting to %s://%s:%d%s \n\n", urlProtocol, clientConnectInfo.address, clientConnectInfo.port, urlPath);

	// Connect with the client info
	lws_client_connect_via_info(&clientConnectInfo);
	if (wsiTest == NULL)
	{
		printf("Error creating the client\n");
		return 1;
	}
	ctx_locker.p_context = ctx;
	ctx_locker.b_locked = 0;
	ctx_locker.p_lws = wsiTest;
	pthread_create(&client_listen_thread_id, NULL, client_run_and_listen, &ctx_locker);
	pthread_create(&client_main_thread_id, NULL, p_cfg->main_thread, &ctx_locker);
	pthread_join(client_listen_thread_id, NULL);
	pthread_join(client_main_thread_id, NULL);

	// Destroy the context
	lws_context_destroy(ctx);
	
	printf("\nDone executing.\n");

	return 0;
}

void *
client_increment_and_transmit(void *p_thread_context)
{
	context_lock *p_context_lock = (context_lock*)p_thread_context;

	printf("Entering transmit thread\n\n");
	while (1)
	{
		lws_callback_on_writable(p_context_lock->p_lws);
		sleep(1);
	}
	pthread_exit(NULL);
}

void *
client_run_and_listen(void *p_thread_context)
{
	context_lock *p_context_lock = (context_lock*)p_thread_context;

	// Main loop runs till bExit is true, which forces an exit of this loop
	while (!bExit)
	{
		// LWS' function to run the message loop, which polls in this example every 50 milliseconds on our created context
		lws_service(p_context_lock->p_context, 50);
	}
	lws_context_destroy(p_context_lock->p_context);
	pthread_exit(NULL);
}

void
minimal_websocket_set_data(minimal_websocket *p_self, unsigned char *p_data, size_t n_bytes)
{
    p_self->p_transmit_data->p_data = p_data;
    p_self->p_transmit_data->n_bytes = n_bytes;
}