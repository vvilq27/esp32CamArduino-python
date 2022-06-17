import serial
import time


s = serial.Serial('COM3', 500000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

tStart = 0

while True:
	l = s.readline()

	# l = l.decode('latin-1').replace("\n", "n")
	# l = l.replace("\r", "r")
	# w = repr(l)
	# # l = l.replace("\r", "wwww")
	
	# for c in w:
	# 	if c == '\n' or c == 'n':
	# 		print('0', end='')
	# 	else:	
	# 		print(c,end='')

	# print("")


	if l[:5] == b'total':
		print(l)
		print(time.time()-tStart)
		tStart = time.time()


	# print(l)