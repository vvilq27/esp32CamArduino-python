import cv2
import numpy

img = open("grayscale.txt", "r")

data= img.read()

data = data.replace('\n','')

hexDataList = [int(data[i:i+2], 16) for i in range(0, len(data), 2)]
img.close()

randomByteArray = bytearray(hexDataList)
flatNumpyArray = numpy.array(randomByteArray)


grayImage = flatNumpyArray.reshape(120, 160)
grayImage = grayImage.transpose()
grayImage = numpy.flip(grayImage,1)

cv2.imwrite('RandomGray.png', grayImage)


# print(type(int(hexDataList[0],16)))
