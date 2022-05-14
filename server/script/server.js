var http = require('http');
var url = require('url');
var fs = require('fs').promises;
const WebSocketServer = require('websocket').server;
var port = 8000;
var host = 'localhost';
let indexFile;
var data = [];
var web_clients = [];

var count = 0;
/*WebSocketServer.on('message', function(message) {
    message = message.slice(0, 50); // max message length will be 50
    data.push(message);
    console.log(message);
});*/

const requestListener = function(req, res) {
    // here we only handle websocket connections
    // in real project we'd have some other code here to handle non-websocket requests
    console.log(req.headers);
    res.setHeader("Content-Type", "text-html");
    res.writeHead(200);
    res.end(indexFile);
}

const server = http.createServer(requestListener);

fs.readFile(__dirname + "/index.html")
    .then(contents => {
        indexFile = contents;
        server.listen(port, host, () => {
            console.log(`Server is running on http://${host}:${port}`);
        });
    })
    .catch(err => {
        console.error(`Could not read index.html file: ${err}`);
        process.exit(1);
    });

const wsServer = new WebSocketServer({httpServer: server})

wsServer.on('request', function(request) {
    const connection = request.accept(null, request.origin);
    console.log('Connection request received.')
    web_clients.push(connection);
    connection.on('message', function(message) {       
        web_clients.forEach(function(client) {
            client.send(message.binaryData);
        });
        console.log("forwarding " + count);
        count = count + 1;
    });

    connection.on('close', function(reasonCode, description) {
        console.log('Client has disconnected.');
    });
});