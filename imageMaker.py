
'''
img = open("in.jpg", "rb")

data = img.read()

# print(data)

k = 0

with open("dataString.txt", "w+") as stringFile:
	for byte in data:
		# print(int(str(byte), 16) )
		# print(byte)
		stringFile.write(hex(byte)[2:])
		k += 1

		if k%2 == 0:
			stringFile.write(" ")

		if k%16 == 0:
			stringFile.write("\n")

	stringFile.close()

img.close()
f = open("img.jpg", "wb+")

# for h in data:
# 	f.write(h)

f.write(data)

f.close()

'''

img = open("data2.txt", "r")

data= img.read()

print(len(data))

hexDataList = [hex(int(data[i:i+2], 16)) for i in range(0, len(data), 2)]


img.close()

# print(type(int(hexDataList[0],16)))

f = open("img2a.jpg", "wb+")


for byte in hexDataList:
	f.write(int(byte,16).to_bytes(1, byteorder='big'))

f.close()

