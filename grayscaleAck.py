import cv2
import numpy
import serial
import os
import time
from imgUtility import makeImg, displayImg, rotateImage, resizeImage
from grayscaleImgFunction import makeGrayImg
import collections
import re


IMG_WIDTH = 160
IMG_HEIGHT = 120
# IMG_ROWS = 3072
# IMG_ROWS = 1536
COMA_POSITION = 3
PIPE_POSITION = 8
LINE_DATA_LENGTH_NO_NL_CHRS = 200
BARE_LINE_LENGTH_NO_NL_CHRS = LINE_DATA_LENGTH_NO_NL_CHRS + 9
START_OF_DATA_IN_LINE_CHR_INDEX = 9
IMG_ROWS = IMG_HEIGHT*IMG_WIDTH//LINE_DATA_LENGTH_NO_NL_CHRS


class ImgRow:
	def __init__(self, imgIdx, imgRowIdx, data):
		self.imgIdx = imgIdx
		self.imgRowIdx = imgRowIdx
		self.data = data

	def __str__(self):
		return "[" + imgIdx + "-" + imgRowIdx + "] " + data[:10]

def validateLine(line):
	if not (line[COMA_POSITION] == 44 and line[PIPE_POSITION] == 124):
		print("invalid line detected: " + str(line))
		
		return False
	else:
		return True

def getRowFromMcu(rowNumber):
	ser.write(bytes(str(rowNumber), 'utf-8'))

	delay = 0.02

	time.sleep(delay)

	while ser.in_waiting != LINE_DATA_LENGTH_NO_NL_CHRS + 2:
		ser.reset_input_buffer()
		delay += 0.01
		# print("getRowFromMcu | delay increased to {}".format(delay))

		ser.write(bytes(str(rowNumber), 'utf-8'))
		time.sleep(delay)

	row = ser.read(ser.in_waiting)[:-2]
	
	return row

def collectMissingRows(missingRowNumbers):
	round = 0

	for rowNumber in missingRowNumbers:
		round += 1
		if round%100 == 0:
			print(rowNumber,end='\r')
		
		row = getRowFromMcu(rowNumber)

		while len(row) != LINE_DATA_LENGTH_NO_NL_CHRS:
			print("recollecting row {}".format(rowNumber))
			row = getRowFromMcu(rowNumber)

		rows[int(rowNumber)] = row

# ==============================================
# ==============================================
# ==============================================
# ==============================================

# add command controll to camera - resolution, color, brightness etc

print(IMG_ROWS)
ser = serial.Serial('COM6', 1000000 )#, parity=serial.PARITY_EVEN, rtscts=1)
ser.set_buffer_size(rx_size = IMG_ROWS * BARE_LINE_LENGTH_NO_NL_CHRS , tx_size = 2560)

ser.reset_input_buffer()

while True:
	missingRowNumbers = list(range(IMG_ROWS))

	rows = dict()
	rws = []

	ser.write(b'ok')
	time.sleep(0.2)
	bytesInBuff = 0;

	# print(ser.in_waiting)

	# while ser.in_waiting != bytesInBuff:
	# 	bytesInBuff = ser.in_waiting
	# 	time.sleep(0.01)

	
	# print("received bytes in buff: " + str(ser.in_waiting))

	while ser.in_waiting != 0:
		line = ser.readline()
		line = line[:-2]	

		if len(line) != BARE_LINE_LENGTH_NO_NL_CHRS:
			continue

	# check for "," and "|" separators
		lineValid = validateLine(line)

		if not lineValid:
			continue

		data = line[START_OF_DATA_IN_LINE_CHR_INDEX:]
		# actually first index is not needed in that mode
		indexes = line[:START_OF_DATA_IN_LINE_CHR_INDEX-1].decode('latin')

		if len(data) != LINE_DATA_LENGTH_NO_NL_CHRS:
			continue

		try:
			imgIdx, imgRowIdx = indexes.split(',')
			imgRowIdx = int(imgRowIdx)
		except:
			print("cannot split this line: " + str(indexes))
			continue

		try:
			missingRowNumbers.remove(imgRowIdx)
		except:
			print("cannot remove number:" + str(imgRowIdx))

		rows[imgRowIdx] = data

	collectMissingRows(missingRowNumbers)

	imageData = bytearray()

	for l in collections.OrderedDict(sorted(rows.items())):
		imageData += rows[l]

	if len(imageData) != IMG_ROWS * LINE_DATA_LENGTH_NO_NL_CHRS:
		print("len imageData: " + str(len(imageData)))
		continue

	match IMG_ROWS:
		case 96:
			print("show qqvga img")
			makeGrayImg(imageData,120,160)
		# case 768:
		case 384:
			print("show qvga img")
			makeGrayImg(imageData,240,320)
		case 1536:
			print("vga")
			makeGrayImg(imageData,320,480)

		case _:
			print("invalid image type : {}".format(IMG_ROWS))
	

