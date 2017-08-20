import numpy as np
import wave
import os
import sys
import scipy
import matplotlib.pyplot as plt
BLOCK_SIZE_MS = 20

def main():
	filename = sys.argv[1]

	sample_rate = rd.getframerate()
	n_samples = rd.getnframes()
	
	
	block_size = BLOCK_SIZE_MS*sample_rate/1000
	split_array = np.array([[]])
	for i in np.xrange(0,n_samples,block_size):
		f = rd.readframes(2*n_samples)
		pdb.set_trace()
		split_array = np.concatenate((split_array,f),axis=0)

	rd.close()

if __name__ == "__main__":
	main()