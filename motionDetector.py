import cv2
import numpy
import datetime

previousState = []

def detectChange(data):
	# for i in range(24):
	# 	row = []
	# 	for k in range(32):
	# 		row.append(data[i][k])

	# for r in data:
	# 	print(r)

	# print()
	# print()

	result = []

	for i in range(10):
		for k in range(10):
			chunk = data[0+i*24:24+i*24, 0+k*32:32+k*32]

			# print(chunk)
			chunkSum = sum(sum(chunk))
			result.append(chunkSum)
	
	print(result)
