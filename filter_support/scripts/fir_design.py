import os
import sys
import cmath
import math
import numpy as np
import scipy
import scipy.signal as sig
import matplotlib.pyplot as plt

def to_c_array(filename,varname,a):
    f = open(filename, 'w')
    f.write('float %s[%d] = \n{\n'%(varname,2*len(a)))
    for i in np.arange(0,len(a)):
        if i != len(a)-1:
            f.write('    %f, %f,\n'%(a[i].real,a[i].imag))
        else:
            f.write('    %f, %f\n'%(a[i].real,a[i].imag))

    f.write('};')
    f.close()


def fir_lowpass(fc,fs,ord):
	filter_coeff=np.zeros(ord,dtype=complex)
	if (ord % 2 == 1):
		for i in np.arange(-(ord-1)/2,(ord-1)/2+1,1):
			if (i==0):
				filter_coeff[i+(ord-1)/2] = 2.0*fc/fs
			else:
				filter_coeff[i+(ord-1)/2] = math.sin(2.0*np.pi*fc*i/fs)/(np.pi*i)

		#power = np.square(np.abs(filter_coeff))
		#power_acc = np.sum(power)
		#dc_level = math.sqrt(power_acc)
		#filter_coeff *= 1.0/dc_level

	return filter_coeff

def fir_highpass(fc,fs,ord):
    filter_coeff=np.zeros(ord,dtype=complex)
    if (ord % 2 == 1):
        for i in np.arange(-(ord-1)/2,(ord-1)/2+1,1):
            if (i==0):
                filter_coeff[i+(ord-1)/2] = 1-2.0*fc/fs
            else:
                filter_coeff[i+(ord-1)/2] = -1.0*math.sin(2.0*np.pi*fc*i/fs)/(np.pi*i)

        #power = np.square(np.abs(filter_coeff))
        #power_acc = np.sum(power)
        #dc_level = math.sqrt(power_acc)
        #filter_coeff *= 1.0/dc_level

    return filter_coeff


def main(argv):
    fc = [int(x) for x in argv[1].split(",")]
    type = [str(x) for x in argv[2].split(",")]
    gain = [float(x) for x in argv[3].split(",")] 
    fs = int(argv[4])
    ord = int(argv[5])
    filename = str(argv[6])
    varname = str(argv[7])
    sum_coeff = np.zeros(ord,dtype=complex)

    for i,freq_cutoff in enumerate(fc):

        if type[i] == "low":
            filter_coeff = fir_lowpass(fc[i],fs,ord)
        elif type[i] == "high":
            filter_coeff = fir_highpass(fc[i],fs,ord)
        else:
            print "Unsupported filter type"
        filter_coeff = np.multiply(np.power(10,1.0*gain[i]/20),filter_coeff)
        sum_coeff += filter_coeff

    to_c_array(filename,varname,sum_coeff)

if __name__ == "__main__":
	main(sys.argv)
