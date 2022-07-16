import serial
import numpy as np
import cv2
import time
import datetime
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



ser = serial.Serial('COM5', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

print('start')

picIdx = 0

waitForNewImage()

while True:
	listOfPackets = []
	data = b''


	newImage = False
	printPacketIdxFlag = False
	
	if picIdx % 5 == 0:
		ser.reset_input_buffer()
		waitForNewImage()

	while not newImage:
		line = ser.readline().decode('latin').split(',')[:-1]
		newImage = checkNewImage(line)

		if newImage:
			break;

			# print image number
		if not printPacketIdxFlag:
			print(line[0])

			printPacketIdxFlag = True

				# remove two first index values
		listOfPackets.append(line[2:])

	for integersList in listOfPackets:
		for number in integersList:
			try:
				data += (int(number).to_bytes(1,byteorder='big'))
			except Exception as e:
				print(e)
				break;

	try:
		img = makeImg(data)
		img = rotateImage(img)
		img = resizeImage(img, 2)
		displayImg(img)

		if picIdx % 15 == 0:
			f = open(datetime.datetime.now().strftime("%x") + "/imgPort" + str(round(picIdx/5)) + ".jpg", "wb+")
			

			f.write(data)

			f.close()
		picIdx += 1
	except Exception as e:
		print(e)