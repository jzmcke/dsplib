import numpy as np
import math
import matplotlib.pyplot as plt
from scipy.misc import imsave

FRACTAL_THRESHOLD = 2

XBOUND = (-1.252,-1.248)
YBOUND = (-0.01,0.01)
N_STEPS_X = 500
N_STEPS_Y = N_STEPS_X*(YBOUND[1]-YBOUND[0])/(XBOUND[1]-XBOUND[0])
MAX_COLOURS = 255
CONTOUR_RES = 10
MAX_ITER = CONTOUR_RES*MAX_COLOURS # cap the number of recursive operations in case of convergence

def main():
	RE = np.arange(XBOUND[0],XBOUND[1],1.0*(XBOUND[1]-XBOUND[0])/N_STEPS_X)
	IM = np.arange(YBOUND[0],YBOUND[1],(YBOUND[1]-YBOUND[0])/N_STEPS_Y)
	mesh = [{'rgb_coord':(i,j), 'coord':(x,y), 'iter': 0} for (i,x) in enumerate(RE) for (j,y) in enumerate(IM)]

	rgb = np.zeros((len(RE),len(IM),3), dtype=np.uint8)

	for coord in mesh:
		cplx_coord = coord['coord'][0] + 1j*coord['coord'][1]
		res = 0
		n = 0
		while abs(res) <= FRACTAL_THRESHOLD and n < MAX_ITER:
			res = res ** 2 + cplx_coord
			coord['iter'] += 1
			n += 1

		rgb[coord['rgb_coord'][0]][coord['rgb_coord'][1]][0] = coord['iter']/CONTOUR_RES
		rgb[coord['rgb_coord'][0]][coord['rgb_coord'][1]][1] = 100
		rgb[coord['rgb_coord'][0]][coord['rgb_coord'][1]][2] = (MAX_ITER-coord['iter'])/CONTOUR_RES

	imsave('fract.png',rgb)

if __name__ == "__main__":
	main()