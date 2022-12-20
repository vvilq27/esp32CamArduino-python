import cv2
import numpy
import datetime

from imgUtility import displayImg, resizeImage
from motionDetector import detectChange

def makeGrayImg(data, height, width):

	flatNumpyArray = numpy.array(data)

	grayImage = flatNumpyArray.reshape(height, width)
	# grayImage = grayImage.transpose()
	# grayImage = numpy.flip(grayImage,1)

	grayResized = resizeImage(grayImage, 3)

	detectChange(grayResized)

	name = '151022/' + datetime.datetime.now().strftime("%H:%M:%S").replace(":","_") +".png"
	# cv2.imwrite(name, grayResized)

	displayImg(grayResized)


# print(type(int(hexDataList[0],16)))
