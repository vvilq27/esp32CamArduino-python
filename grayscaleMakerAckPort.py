import cv2
import numpy
import serial
import os
import time
from imgUtility import makeImg, displayImg, rotateImage, resizeImage
from grayscaleImgFunction import makeGrayImg
import collections
import re



class ImgRow:
	def __init__(self, imgIdx, imgRowIdx, data):
		self.imgIdx = imgIdx
		self.imgRowIdx = imgRowIdx
		self.data = data

	def __str__(self):
		return "[" + imgIdx + "-" + imgRowIdx + "] " + data[:10]

def checkNewImage(packet):
	if packet[:7] == 'newimg:':
		return int(packet[7:-2])
	else:
		return False

def waitForNewImage():
	ser.reset_input_buffer()
	while True:
		packet = ser.readline()

		if len(packet)>20:
			continue
		else:
			packet = packet.decode('utf-8')
			newImgLength = checkNewImage(packet)
			
			if newImgLength:
				return newImgLength

def getRowFromMcu(rowNumber):
	ser.write(bytes(str(n), 'utf-8'))
	time.sleep(0.15)
	row = ser.readline()[:-2]
	# print(len(row) , end=' ')
	# print(row)

	return row


ser = serial.Serial('COM3', 1000000, timeout=0.01 )#, parity=serial.PARITY_EVEN, rtscts=1)
ser.set_buffer_size(rx_size = 100000, tx_size = 256)


# bytesCountToRead = waitForNewImage()

# print(bytesCountToRead)

ser.reset_input_buffer()

missingRowNumbers = list(range(768))

# missingRowNumbers.remove(1)
# missingRowNumbers.remove(5)
# print(missingRowNumbers)

rows = dict()
rws = []

# while ser.in_waiting < 70000:
# 	ser.write(b'ok')
# 	time.sleep(4)
	# print(ser.read(4094))
	# print(ser.in_waiting)

ser.write(b'ok')
time.sleep(1.5)


print("pic collected")

while ser.in_waiting != 0:
	line = ser.readline()
	line = line[:-2]

	# lineValidityCheck = re.match("\d\d\d,\d\d\d\|", line)

	# if lineValidityCheck == None:
	# 	print("invalid line detected: ", end="")
	# 	print(line)
	# 	continue		

	if len(line) < 40:
		continue


	if not (line[3] == 44 and line[7] == 124):
		print("invalid line detected: ", end="")
		print(line)
		continue


	data = line[8:]
	# actually first index is not needed in that mode
	indexes = line[:7].decode('latin')

	if len(data) != 100:
		continue


	imgIdx, imgRowIdx = indexes.split(',')

	try:
		imgRowIdx = int(imgRowIdx)
	except:
		continue


	try:
		missingRowNumbers.remove(imgRowIdx)
	except:
		print("cannot remove number: ", end = "")
		print(imgRowIdx)

	# row = ImgRow(imgIdx, imgRowIdx, data)

	# print(len(data), end = ' ')
	# print(row)

	rows[imgRowIdx] = data

print(missingRowNumbers)

# for l in rows:
# 	print(l, end='. ')
# 	print(rows[l])


for n in missingRowNumbers:
	print(n, end = ' is missing, retry')
	row = getRowFromMcu(n)

	while len(row) != 100:
		print("recollecting row {}".format(n))
		row = getRowFromMcu(n)

	rows[int(n)] = row


result = bytearray()

for l in collections.OrderedDict(sorted(rows.items())):
	result += rows[l]
	# print(l, end='. ')
	# print(len(rows[l]))
	# print(rows[l])

print(len(result))

makeGrayImg(result)


time.sleep(5)



# for i in range(9):
# 	data = ser.readline()
# 	print(data)


while True:

	# for i in range(9):
	# 	ser.write(hex(i))
	# 	data = ser.readline()
	# 	print(data)

	# time.sleep(1)
	break

	# imgBytesArr = []

	# idx = 50

	# while idx:
	# 	line = ser.read(4000)
	# 	imgBytesArr.append(line)
	# 	idx = idx -1
	# 	# print(idx)

	# nlFlag = False

	# # for line in imgBytesArr:
	# for c in data:
	# 	if c=='\r':
	# 		nlFlag = True
	# 		continue

	# 	if c == '\n' and nlFlag:
	# 		print()
	# 		nlFlag = False

	# 	nlFlag = False

	# 	try:
	# 		print(chr(c), end='')
	# 	except:
	# 		print("", end='')

	

	# input("Press Enter to continue...")

	'''
	print('pic done')

	randomByteArray = bytearray(data)
	flatNumpyArray = numpy.array(randomByteArray)


	grayImage = flatNumpyArray.reshape(120*2, 160*2)
	grayImage = grayImage.transpose()
	grayImage = numpy.flip(grayImage,1)

	# cv2.imwrite('RandomGray.png', grayImage)

	displayImg(grayImage)

	ser.readline()
	ser.readline()
	'''
