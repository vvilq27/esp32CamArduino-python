import cv2
import numpy
import serial
import os
import time
from imgUtility import makeImg, displayImg, rotateImage, resizeImage

import collections

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


ser = serial.Serial('COM3', 1000000, timeout=0.01 )#, parity=serial.PARITY_EVEN, rtscts=1)

# bytesCountToRead = waitForNewImage()

# print(bytesCountToRead)

ser.flush()

rowNumbers = list(range(20))

# rowNumbers.remove(1)
# rowNumbers.remove(5)
# print(rowNumbers)

rows = dict()
rws = []

while ser.in_waiting < 2200:
	ser.write(b'ok')
	time.sleep(0.4)



while ser.in_waiting != 0:
	line = ser.readline()
	line = line.strip().decode('latin')

	try:
		indexes, data = line.split('|')
	except:
		continue
	imgIdx, imgRowIdx = indexes.split(',')

	rowNumbers.remove(int(imgRowIdx))

	# row = ImgRow(imgIdx, imgRowIdx, data)

	# print(len(data), end = ' ')
	# print(row)

	rows[int(imgRowIdx)] = data

	# for r in rows:
	# 	print(r)


# for r in rows:
# 	print(id(r))
# 	print(r)


for l in rows:
	print(l, end='. ')
	print(rows[l])


for n in rowNumbers:
	print(n, end = ' ')
	ser.write(bytes(str(n), 'utf-8'))
	time.sleep(0.1)
	row = ser.readline().decode('latin').strip()
	rows[int(n)] = row


print()

for l in collections.OrderedDict(sorted(rows.items())):
	print(l, end='. ')
	print(rows[l])



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
