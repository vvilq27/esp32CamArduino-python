import numpy as np
import cv2
import time
import imutils


def makeImg(imgBts):
	d = bytes.fromhex(imgBts.upper())
	img = cv2.imdecode(np.frombuffer(d, np.uint8), -1)

	return img

def makeImg2(imgBts):
	img = cv2.imdecode(np.frombuffer(imgBts, np.uint8), -1)

	return img

def displayImg(img):
	cv2.imshow('Input', img)

	c = cv2.waitKey(1)

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

data = b''

f = open('intBytes.txt', 'r')

strDataArray = f.readlines()
# for c in [chr(int(i)) for i in strDataArray[0].split(',')]:
# 	print(c.hex(), end=',')

# print(int(strDataArray[0][-2]))


for line in strDataArray:#[:-2]:
	numbers = line.split(',')[2:-1]

	if len(numbers) != 30:
		print(len(numbers))
		print(numbers)


	for n in numbers:
		data += (int(n).to_bytes(1,byteorder='big'))

try:
	img = makeImg2(data)
	img = rotateImage(img)
	img = resizeImage(img, 2)
	displayImg(img)
	
	# f.close()
except Exception as e:
	print(e)

	# time.sleep(2)

time.sleep(4)


# print(data)