import cv2
import numpy
import serial
import os

def checkNewImage(packet):
	if packet[:7] == 'newimg:':
		print(packet)
		return True
	else:
		return False

def waitForNewImage():
	# ser.reset_input_buffer()
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
data = ser.read(76800)
print('pic done')

randomByteArray = bytearray(data)
flatNumpyArray = numpy.array(randomByteArray)


grayImage = flatNumpyArray.reshape(120*2, 160*2)
grayImage = grayImage.transpose()
grayImage = numpy.flip(grayImage,1)

cv2.imwrite('RandomGray.png', grayImage)

os.startfile('.\\RandomGray.png')


# print(type(int(hexDataList[0],16)))
