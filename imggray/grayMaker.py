import cv2
import time
from PIL import Image
import io
import numpy as np

img = open("pic20.txt", "rb")
data = img.read()

bmpdata = []

for n in data:
	bmpdata.append(int(n))
	# print(n)

i = np.array(bmpdata)


# bmp = Image.open(io.BytesIO(data))
im = Image.fromarray(i.reshape(120,160).astype(np.uint8))

# im.show()
im.save('pil.bmp')



# img.close()

# randomByteArray = bytearray(data)
# flatNumpyArray = numpy.array(randomByteArray)


# grayImage = flatNumpyArray.reshape(120, 160)
# grayImage = grayImage.transpose()
# grayImage = numpy.flip(grayImage,1)

# cv2.imwrite('RandomGrayz.png', grayImage)


# img = Image.open('RandomGrayz.png')
# file_out = "test1.bmp"
# img.save(file_out)



# cv2.imshow('Input', grayImage)

# c = cv2.waitKey(1)
# time.sleep(2)