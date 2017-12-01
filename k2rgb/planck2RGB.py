#!/usr/bin/env python
'''
temperature from planck's equation to RGB values
'''

from math import log, e
import matplotlib.pyplot as plt
from scipy.constants import c,h, k


def planck(wavelenght, T):
	wvl=wavelenght*1e-9
	B=2*h*(c**2)/((wvl**5)*(e**(h*c/(wvl*k*T))-1))
	return B #if B*1e-12 units kW·str-1·m-2·nm-1 else W·str-1·m-3

def clamp(x, smallest, largest):
    if x< smallest:
        return smallest
    if x> largest:
        return largest
    return x

def planck2RGB(wavelenghts, T):
	answer=[]
	for wvl in wavelenghts:
		value=log(planck(wvl, T))
		if value<0:
			value=0
		answer.append(value)
	return answer



if __name__ == "__main__":
	show = 2
	if show == 0:
		for temperature in range(3000, 7000, 1000):
			x=[]
			y=[]
			for i in range(150, 1500):
				x.append(i)
				y.append(planck(i, temperature)*1e-12)
			linelable="temperature: "+str(temperature)
			plt.plot(x, y, label=linelable)
		plt.legend()
		plt.show()
		
	
	if show ==1:
		wavelenghts=[420, 530, 680]
		for wvl in wavelenghts:
			y=[]
			x=[]
			for temperature in range(600, 6000, 10):
				x.append(temperature)
				y.append(log(planck(wvl, temperature)))
			lnlabel=str(wvl)+' nm'
			plt.plot(x, y, label=lnlabel)
		plt.legend()
		plt.show()
	if show==2:
		wavelenghts=[420, 530, 680]
		x=[]
		r=[]
		g=[]
		b=[]
		for temperature in range(600, 6000, 10):
			x.append(temperature)
			tmpr, tmpg, tmpb=planck2RGB(wavelenghts, temperature)
			r.append(tmpr)
			g.append(tmpg)
			b.append(tmpb)
		plt.plot(x, r, color='r')
		plt.plot(x, g, color='g')
		plt.plot(x, b, color='b')
		plt.show()
	
