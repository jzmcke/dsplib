import pickle
import os
import blob_read as br

log_dict = dict()
def parse_log(log_dir):
    for f in os.listdir(log_dir):
        with open(os.path.join(log_dir, f), 'rb') as pkl_rd:
            data = pickle.load(pkl_rd)
        
        for blob in data['data']:
            ip_addr = blob[:128].decode("utf-8").strip("\x00")
            if not (ip_addr in log_dict.keys()):
                log_dict[ip_addr] = []
            
            blob_deserialized, _ = br.blob_read_node_tree(blob[128:])

            blob_unpacked = br.blob_minimal(blob_deserialized)

            log_dict[ip_addr].append(blob_unpacked)

        import pdb; pdb.set_trace()

def console():
    import argparse
    parser = argparse.ArgumentParser(description="Log to unpack")
    parser.add_argument("log_dir", help="Path to log directory")
    opts = parser.parse_args()

    parse_log(opts.log_dir)


if __name__ =='__main__':
    console()