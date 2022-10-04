import serial
import numpy as np
import cv2
import time
import datetime
import os
import re
# import imutils

from imgUtility import makeImg, displayImg, rotateImage, resizeImage

def checkNewImage(packet):
	# print(packet[:5])
	# if packet[:5] == ['115', '105', '122', '101', '58']: # size: string
	# if packet[:5] == ['s','i','z','e',':']:

	if packet[:5] == 'size:':
		return True
	else:
		return False

def checkIfLastPacket(packet):
	if packet == 'img cplt\r\n':
		return True
	else:
		return False

def waitForNewImage():
	ser.reset_input_buffer()
	while True:
		# packet = ser.readline().decode('latin').split(',')[:-1]
		
		# packet = ser.readline()

		packet = ser.readline().decode('utf-8').split(',')

		if len(packet) == 1:
			if checkNewImage(packet[0]):
				break

		
		# for c in packet:
		# 	if c == 10:
		# 		print('10,', end='')
		# 	if c == 13:
		# 		print('13,', end='')
		# 	else:
		# 		print(c, end=',')

		
		# if(len(packet) != 30):
		# 	print(packet)

		# if len(packet) == 32:
		

todayDate = datetime.datetime.now().strftime("%x").replace('/','-')
path = "C:/Users/aro/Documents/python/camera_jpeg_conv/" + todayDate
# path = C:\Users\aro\Documents\python\camera_jpeg_conv

if os.path.isdir(path) == False:
	os.mkdir(path)


# folderImages = os.listdir(path)
# print(folderImages)
# imageIndexes = []

# for i, imgName in enumerate(folderImages):
# 	index = re.search(path + "(\d+)", imgName)
# 	print(index)
# 	imageIndexes.append(int(index.group(1)))

# picIdx = max(imageIndexes) +1


ser = serial.Serial('COM3', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

picIdx = 0

# waitForNewImage()

while True:
	listOfPackets = []
	data = b''

	# ser.write(str.encode('d'))

	newImage = False
	lastPacket = False

	if picIdx % 5 == 0:
		ser.reset_input_buffer()
		waitForNewImage()

	while not lastPacket:
		line = ser.readline().decode('latin').split(',') #remove /r/n as last element of array

		lastPacket = False

		if len(line) == 1:
			# newImage = checkNewImage(line[0])
			lastPacket = checkIfLastPacket(line[0])

			if lastPacket:
				print('last packet rcv')
				lastPacket = True
				break;


		listOfPackets.append(line[1:-1])

	print('data collection finished')


	for p in listOfPackets:
		print(p)

	print(len(listOfPackets))

	for integersList in listOfPackets:
		for number in integersList:
			try:
				data += (int(number).to_bytes(1,byteorder='big'))
			except Exception as e:
				print(e)
				break;

	# print(len(data))


	try:
		img = makeImg(data)
		img = rotateImage(img)
		# img = resizeImage(img, 1)
		displayImg(img)

		if picIdx % 3 == 0:
			f = open(todayDate + "/" + datetime.datetime.now().strftime("%H:%M:%S").replace(':','.') + ".jpg", "wb+")
			

			f.write(data)

			f.close()
		picIdx += 1
	except Exception as e:
		print(e)
