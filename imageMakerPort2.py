import serial
import numpy as np
import cv2
import time
# import imutils

from imgUtility import makeImg, displayImg, rotateImage, resizeImage

def checkNewImage(packet):
	if packet[:5] == ['115', '105', '122', '101', '58']: # size: string
		return True
	else:
		return False

def waitForNewImage():
	while True:
		packet = ser.readline().decode('latin').split(',')[:-1]
		
		if len(packet) == 32:
			if checkNewImage(packet):
				break



ser = serial.Serial('COM12', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

print('start')
lines = 240

while True:
	packets = []
	data = b''

	waitForNewImage()

	for i in range(lines):
		packets.append(ser.readline())

	# ser.close()

	printPacketIdxFlag = False

	for p in packets:
		if not printPacketIdxFlag:
			print(p[:3])

			printPacketIdxFlag = True

		# remove first two index values(pic idx and packet idx) and new line char
		p = p.decode('latin').split(',')[2:-1]
		if not checkNewImage(p):
			# print(p)
			for number in p:
				data += (int(number).to_bytes(1,byteorder='big'))

		else:
			break

	try:
		img = makeImg(data)
		img = rotateImage(img)
		img = resizeImage(img, 2)
		displayImg(img)
	except Exception as e:
		print(e)