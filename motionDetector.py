import cv2
import numpy as np
import datetime

previousState = []

def detectChange(data):
	result = []

	blocks = [[[] for g in range(10)] for k in range (10)]

	for i in range(len(data)):
		for k in range(10): 			
			blocks[int(i/12)][k].append(data[i][0+16*k:16+16*k])
			
	# print(
	# 	sum(
	# 		list(
	# 			map(
	# 				lambda x: int(x), 
	# 				np.concatenate(blocks[0][0]) 
	# 			)
	# 		)
	# 	)
	# )


	for i, row in enumerate(blocks):
		for imgPart in row:
			imgPartSum = sum(
							list(
								map(
									lambda x: int(x), 
									np.concatenate(imgPart) 
								)
							)
						)
			result.append(imgPartSum)

	return result