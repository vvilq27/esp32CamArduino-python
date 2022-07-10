import numpy as np
import cv2
import imutils

def makeImg(imgBts):
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