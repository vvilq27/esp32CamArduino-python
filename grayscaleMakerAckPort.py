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

def validateLine(line):
	if not (line[3] == 44 and line[7] == 124):
		print("invalid line detected: ", end="")
		print(line)
		
		return False
	else:
		return True

def getRowFromMcu(rowNumber):
	ser.write(bytes(str(n), 'utf-8'))

	delay = 0.05

	time.sleep(delay)

	while ser.in_waiting != 102:
		ser.reset_input_buffer()
		delay += 0.01
		print("getRowFromMcu | delay increased")

		ser.write(bytes(str(n), 'utf-8'))

	row = ser.read(ser.in_waiting)[:-2]
	
	return row



# ==============================================
# ==============================================
# ==============================================
# ==============================================


ser = serial.Serial('COM3', 1000000, timeout=0.01 )#, parity=serial.PARITY_EVEN, rtscts=1)
ser.set_buffer_size(rx_size = 100000, tx_size = 256)

ser.reset_input_buffer()

missingRowNumbers = list(range(768))

rows = dict()
rws = []

# while ser.in_waiting < 70000:
# 	ser.write(b'ok')
# 	time.sleep(4)
	# print(ser.read(4094))
	# print(ser.in_waiting)

ser.write(b'ok')
time.sleep(1)
print(ser.in_waiting)
print("pic collected")

while ser.in_waiting != 0:
	line = ser.readline()
	line = line[:-2]	

	if len(line) != 108:
		continue

# check for "," and "|" separators
	lineValid = validateLine(line)

	if not lineValid:
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

	rows[imgRowIdx] = data

print(missingRowNumbers)

for n in missingRowNumbers:
	print(n, end = ' is missing, retry\n')
	row = getRowFromMcu(n)

	while len(row) != 100:
		print("recollecting row {}".format(n))
		row = getRowFromMcu(n)

	rows[int(n)] = row


result = bytearray()

for l in collections.OrderedDict(sorted(rows.items())):
	result += rows[l]

makeGrayImg(result)


time.sleep(3)
