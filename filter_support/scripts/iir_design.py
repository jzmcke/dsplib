import os
import sys
import cmath
import math
import numpy as np
import scipy
import scipy.signal as sig
import matplotlib.pyplot as plt

def to_c_array(filename,varname,a,b_append=0):
    if b_append == 1:
        type = 'a+'
    else:
        type = 'w'

    f = open(filename, type)
    f.write('float %s[%d] = \n{\n'%(varname,2*len(a)))
    for i in np.arange(0,len(a)):
        if i != len(a)-1:
            f.write('    %.18f, %.18f,\n'%(a[i].real,a[i].imag))
        else:
            f.write('    %.18f, %.18f\n'%(a[i].real,a[i].imag))

    f.write('};\n\n')
    f.close()

def main(argv):
    
    type = str(argv[2])
    gain_db = float(argv[3])
    fs = int(argv[4])
    ord = int(argv[5])
    filename = str(argv[6])
    varname = str(argv[7])
    sum_coeff = np.zeros(ord,dtype=complex)
    fc = tuple([float(x)/(fs/2) for x in argv[1].split(',')])
    if type == 'high':
        type = 'highpass'
    elif type == 'low':
        type = 'lowpass'


    filter_coeff_n,filter_coeff_d = sig.butter(ord,fc,btype=type,analog=False,output='ba')
    print (filter_coeff_n,filter_coeff_d)
    impulse = np.zeros(882)
    impulse[0] = 1;
    out = sig.lfilter(filter_coeff_n,filter_coeff_d,impulse)
    plt.plot(out)
    plt.title('Impulse response')
    plt.show()

    #filter_coeff_n = np.multiply(np.power(10,1.0*gain_db),filter_coeff_n)

    to_c_array(filename,varname+'_n',filter_coeff_n,b_append=0)
    to_c_array(filename,varname+'_d',filter_coeff_d[1:],b_append=1) # API assumes y[n] coefficient = 1
    

if __name__ == "__main__":
	main(sys.argv)
