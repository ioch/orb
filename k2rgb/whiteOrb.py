#!/usr/bin/env python
'''
lets try to play some whites on orb
'''
import serial
from time import sleep
from planck2RGB import planck2RGB

def formatStr(r, g, b):
	string=('C '+str(r)+' '+str(g)+' '+str(b)+'\r').encode('ascii')
	return string

def readComm():
	buff=[]
	while not port.in_waiting:
		sleep(0.1)
	while port.in_waiting>0:
		buff.extend(port.read(port.in_waiting))
	return bytes(buff[:-1]).decode("ascii")


port=serial.Serial('/dev/rfcomm0', baudrate=9600, bytesize=8, stopbits=1,timeout=1)
sleep(0.5)
port.write('M 2\r'.encode('ascii'))
sleep(0.1)
print(readComm())
scale =200
try:
	
	for i in range(600, 6000, 4):
		#~ r, g, b=kelvin2RGB(i)
		r, g, b = planck2RGB([680, 530, 420], i)
		r=r/24.75*scale
		g=g/33.11*scale
		b=b/34.75*scale
		string=formatStr(r, g, b)
		port.write(string)
		sleep(0.2/(i/1000))
		print("kelvin:", i, readComm())

except KeyboardInterrupt:
	pass
port.write('M 1\r'.encode('ascii'))
sleep(0.1)
print(readComm())
port.close()
