import cv2
import numpy
import datetime

from imgUtility import displayImg, resizeImage
from motionDetector import detectChange

def makeGrayImg(data, height, width, lastState):
	# if 'lastState' not in locals():
	# 	lastState = [0 for i in range(100)]
	# 	print("initialize last state variable")

	flatNumpyArray = numpy.array(data)

	grayImage = flatNumpyArray.reshape(height, width)
	# grayImage = grayImage.transpose()
	# grayImage = numpy.flip(grayImage,1)

	grayResized = resizeImage(grayImage, 3)

	# for block in grayImage:
	# 	print(block)

	currentState = detectChange(grayImage)

	changedBlocks = 0

	for i, val in enumerate(currentState):
		change = abs(val-lastState[i])
		if(lastState[i]!= 0):
			diff = format(val/lastState[i], '.2f')
			if abs(float(diff) - 1) > 0.15:
				changedBlocks += 1

	if changedBlocks > 3:
		print("MOVEMENT!")


	name = '150123/' + datetime.datetime.now().strftime("%H:%M:%S").replace(":","_") +".png"
	# cv2.imwrite(name, grayResized)

	displayImg(grayResized)

	return currentState


# print(type(int(hexDataList[0],16)))