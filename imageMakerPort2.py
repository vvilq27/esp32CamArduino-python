import serial
import numpy as np
import cv2
import time
# import imutils

# from imgUtility import makeImg2, displayImg, rotateImage, resizeImage

# def makeImg2(imgBts):
# 	picData = np.frombuffer(imgBts, np.uint8)

# 	img = cv2.imdecode(picData, -1)

# 	return img

# def displayImg(img):
# 	cv2.imshow('Input', img)

# 	c = cv2.waitKey(1)


def makeImg2(imgBts):
	img = cv2.imdecode(np.frombuffer(imgBts, np.uint8), -1)

	return img

def displayImg(img):
	cv2.imshow('Input', img)

	c = cv2.waitKey(1)

	time.sleep(0.2)

def rotateImage(image):
	(h, w) = image.shape[:2]

	(cX, cY) = (w // 2, h // 2)

	# rotate our image by 45 degrees around the center of the image
	M = cv2.getRotationMatrix2D((cX, cY), -90, 1.0)
	rotatedImage = cv2.warpAffine(image, M, (w, h))

	return rotatedImage

def resizeImage(image, ratio):
	(h, w) = image.shape[:2]
	
	dim = (w*ratio, h*ratio)
	resized = cv2.resize(image, dim, interpolation = cv2.INTER_AREA)

	return resized

def checkNewImage(packet):
	# if packet[-1] == '0' and packet[-2] == '0' and packet[-3] == '0' and packet[-4] == '0' :
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



ser = serial.Serial('COM6', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

print('start')
lines = 240

while True:
	packets = []
	data = b''

	waitForNewImage()

	for i in range(lines):
		packets.append(ser.readline())

	# ser.close()

	for p in packets:
		p = p.decode('latin').split(',')[2:-1]
		if not checkNewImage(p):
			# print(p)
			for number in p:
				data += (int(number).to_bytes(1,byteorder='big'))

		else:
			break

	try:
		img = makeImg2(data)
		img = rotateImage(img)
		img = resizeImage(img, 2)
		displayImg(img)
	except Exception as e:
		print(e)