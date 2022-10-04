import cv2
import numpy
import serial
import os
import time
from imgUtility import makeImg, displayImg, rotateImage, resizeImage


def checkNewImage(packet):
	if packet[:7] == 'newimg:':
		print(packet)
		return True
	else:
		return False

def waitForNewImage():
	ser.reset_input_buffer()
	while True:
		packet = ser.readline()

		if len(packet)>20:
			continue
		else:
			packet = packet.decode('utf-8')
			if checkNewImage(packet):
				break


ser = serial.Serial('COM3', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

waitForNewImage()
while True:
	data = ser.read(76800+6144+3*256*2)
	print('pic done')

	randomByteArray = bytearray(data)


# //unreliable, buffer has corrupted data sometimes, without data check image will be often scattered
	for i in range(0, 3*256):
		print(randomByteArray[i*100:i*100+8])

		for k in range(0,8):
			
			# print(chr(randomByteArray[i*100]), end=',')
			del randomByteArray[i*100]

		print()

		# print(randomByteArray[i*100+100])
		del randomByteArray[i*100+100]
		del randomByteArray[i*100+100]
		# print(randomByteArray[i*100+95:i*100+105])


	# print(randomByteArray[:200])''

	print(len(randomByteArray))

	# break

	flatNumpyArray = numpy.array(randomByteArray)


	grayImage = flatNumpyArray.reshape(120*2, 160*2)
	grayImage = grayImage.transpose()
	grayImage = numpy.flip(grayImage,1)

	# cv2.imwrite('RandomGray.png', grayImage)

	displayImg(grayImage)

	ser.readline()
	ser.readline()
