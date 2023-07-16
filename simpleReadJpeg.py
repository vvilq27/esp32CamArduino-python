# read every 100 bytes
# check for 0xff,0xd8
# read until 0xff,0xd9

import serial
import numpy as np
import cv2

ser = serial.Serial('COM6', 1000000 )#, parity=serial.PARITY_EVEN, rtscts=1)

imgFound = False
buff = []

start = 0 
end = 0

collectingImage = False

while imgFound is not True:
	line = ser.read(100)
	# print(line, end="\r\n")

	if collectingImage is False:
		for n in range(len(line)-1):
			if line[n] == 255 and line[n+1] == 216:
				collectingImage = True
				# buff += line
				break

	else:
		# buff += line

		for n in range(len(line)-1):
			if line[n] == 255 and line[n+1] == 217:
				collectingImage = False
				break

		if collectingImage is False:
			imgFound = True

print('start')

imgFound = False
collectingImage = False

while imgFound is not True:
	line = ser.read(100)
	# print(line, end="\r\n")

	if collectingImage is False:
		for n in range(len(line)-1):
			if line[n] == 255 and line[n+1] == 216:
				print('found image')
				start = n
				collectingImage = True
				buff += line
				break

	else:
		buff += line
		print("add line")

		for n in range(len(line)-1):
			if line[n] == 255 and line[n+1] == 217:
				collectingImage = False
				end = n +2
				break

		if collectingImage is False:
			imgFound = True

print(buff[start:len(buff)- (100 - end)])

data = buff[start:len(buff)- (100 - end)]


img = cv2.imdecode(np.frombuffer(bytearray(data), np.uint8), -1)
cv2.imshow('Input', img)
c = cv2.waitKey(1)

while True:
	pass
	