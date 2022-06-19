from cmath import e
import websocket
import rel
import concurrent.futures
import blob_read as br
import time
from datetime import datetime
import pickle
import signal
import sys
import os

b_msg_lock = False
times = []
msgs = []
queue = []
file_count = 0

start_time = time.time()
log_dict = dict()
folder_name = 'log_' + datetime.now().strftime("%m_%d_%Y-%H_%M_%S")
os.mkdir(folder_name)


def on_message(ws, message):
    global msgs
    global file_count
    global times
    # print('Msg received. len msgs: ' + str(len(msgs)))

    msgs.append(message)
    times.append(time.time() - start_time)

    if len(msgs) > 1000:
        with open(os.path.join(folder_name, f"log_{file_count}.pkl"),'wb') as pkl_wt:
            pickle.dump({'data': msgs, 'times': times}, pkl_wt)
        file_count += 1
        msgs = []
        times = []

def on_error(ws, error):
    # print(error)
    pass

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")
    

def on_open(ws):
    print("Opened connection")

# def process_log(i):
#     global b_msg_lock
#     print('hello')
#     while (1):
#         if len(msgs) > 0 and len(queue) == 0 and not b_msg_lock:
#             print(len(queue))
#             b_msg_lock = True
#             msgs = msgs + queue
#             b_msg_lock = False
#             queue = []
#         print("Foo")
#         if len(queue) > 0:
#             new_blob = queue.pop(0)
#             ip_addr = str(new_blob[:128])
#             if not (ip_addr in log_dict.keys()):
#                 log_dict[ip_addr] = []
            
#             blob, node_name, _ = br.blob_read_node_tree(new_blob[128:])
            
#             log_dict[ip_addr].append({node_name: blob})
            
def close_up_shop(signal, frame):
    global msgs
    global times
    print('Logging ' + str(len(msgs)) + ' packets of data')
    with open(os.path.join(folder_name, f"log_{file_count}.pkl"),'wb') as pkl_wt:
        pickle.dump({'data': msgs, 'times': times}, pkl_wt)      
    msgs = []
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, close_up_shop)
    print('Press CTRL+C')
    
    # websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://192.168.50.115:8000",
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close)


    ws.run_forever()  # Set dispatcher to automatic reconnection
    
    while True:
        pass
    

    
