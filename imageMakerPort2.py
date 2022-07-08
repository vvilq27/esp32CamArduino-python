import serial


# may need to add pic id

def checkNewImage(packet):
	# if packet[-1] == '0' and packet[-2] == '0' and packet[-3] == '0' and packet[-4] == '0' :
	if packet[:5] == "size:"
		return True
	else:
		return False

ser = serial.Serial('COM6', 1000000, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

newImg = False

# imageBytes = ''

while not newImg:
	packet = ser.readline().decode('latin').split(',')[:-1]
	

	if len(packet) == 32:
		# print(packet[-1], packet[-2], packet[-3], packet[-4])
		if checkNewImage(packet):
			newImg = True


# while True:
newImg = False

# why bytearray?
imageBytesTab = bytearray()
packets = []

# packet = packet.decode('utf-8')
packet = ''
lines = 322

tab = []

for i in range(lines):
	packets.append(ser.readline())

ser.close()

for p in packets:
	p = p.decode('latin').split(',')[:-1]
	if not checkNewImage(p):
		print(p)
	else:
		print("last packet")
		print(p)
		break

	

	# f = open("imgPort.jpg", "wb+")

	# f.write(imageBytesTab)

	# f.close()