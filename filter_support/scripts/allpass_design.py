import sys
import os
from scipy.fftpack import fft
import cmath
import math
import matplotlib.pyplot as plt
import numpy as np


def main():

	sample_rate = int(sys.argv[3])
	block_size_ms = int(sys.argv[4])
	real = float(sys.argv[1])
	imag = float(sys.argv[2])
	N = sample_rate*block_size_ms/1000
	wn,Hw = single_pair(real,imag,N)


	magHw = abs(Hw)
	pHw = np.angle(Hw)

	# Plot the magnitude and phase responses
	plt.plot(wn,magHw)
	plt.title('Magnitude response')
	plt.ylabel('Magnitude (linear)')
	plt.xlabel('Frequency')
	plt.figure()
	plt.plot(wn,np.unwrap(pHw))
	plt.title('Phase response')
	plt.ylabel('Phase (radians)')
	plt.xlabel('Frequency')
	plt.show()

def single_pair(real,imag,N):

	w_array = np.arange(0,2*math.pi,2*math.pi/N)
	Hw = np.zeros(len(w_array),dtype=complex)
	for i in np.arange(0,len(w_array)):
		Hw[i] = (-real + 1j*imag + cmath.exp(-1j*w_array[i]))/(1-(real + 1j*imag)*cmath.exp(-1j*w_array[i]))

	
	return (w_array,Hw)

if __name__ == "__main__":
	main()