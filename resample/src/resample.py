import scipy.fftpack as fft
import numpy as np
import wave
import sys
import struct
import matplotlib.pyplot as plt

def resample(in_audio, fs_in, fs_out):
    if fs_out > fs_in:
        up_factor = fs_out/fs_in
        rs_fft = np.zeros(up_factor*len(in_audio),dtype=complex)
        in_fft = fft.fft(in_audio)
        rs_fft[:(len(in_fft)/2)] = in_fft[:len(in_fft)/2]
        rs_fft[-(len(in_fft)/2):] = in_fft[len(in_fft)/2:]
        out = fft.ifft(rs_fft)

    return out.real

if __name__ == "__main__":
    in_file = sys.argv[1]
    out_file = sys.argv[2]
    fs_out = sys.argv[3]

    rd = wave.open(in_file,  'r')
  
    fs_in = rd.getframerate()
    n = rd.getnframes()
    in_audio = rd.readframes(n)
    unpack_audio = np.zeros((n,1))
    for i in range(len(in_audio)/2):
        unpack_audio[i] = struct.unpack('h', in_audio[2*i:2*i+2])

    unpack_audio = 1.0*unpack_audio / 32768
    unpack_audio = unpack_audio.reshape((1,len(unpack_audio)))[0]
    rd.close()
    
    
    out_audio = resample(unpack_audio, int(fs_in), int(fs_out))
    wt = wave.open(out_file, 'w')
    wt.setframerate(fs_out)
    wt.setnchannels(1)
    wt.setsampwidth(2)
    out_packed = []

    
    out_audio = [int(sample) for sample in out_audio * np.power(2,15)]

    for i in range(len(out_audio)/2):
        out_packed.append(struct.pack('h',out_audio[i]))

    import pdb
    pdb.set_trace()
    wt.writeframes(out_audio)
    wt.close()
