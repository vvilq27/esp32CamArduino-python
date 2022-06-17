import serial


# may need to add pic id


ser = serial.Serial('COM10', 115200, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

tab = []

newImg = False

imageBytes = ''

while not newImg:
	packet = ser.readline()
	newImg = packet[:8] == b'received'
	continue

while True:

	newImg = False

	packet = packet.decode('utf-8')

	packetParts = packet.split(' ')[4:6]
	print(packetParts)

	while not newImg:
		packet = ser.readline()
		newImg = packet[:8] == b'received'
		if not newImg:
			tab.append(packet)
		continue

	# ser.close()

	# //check if last array has ffd9
	# for tabPacket in tab:
	for i in range(len(tab)):
		if i == len(tab) - 1:
			s = tab[i].decode('utf-8')
			s = s[2:-2]
			s = s.rstrip("0")		

			imageBytes += s 

		else :
			imageBytes += tab[i].decode('utf-8')[2:-2]

	hexDataList = [hex(int(imageBytes[i:i+2], 16)) for i in range(0, len(imageBytes), 2)]
	print(len(hexDataList))

	f = open("imgPort.jpg", "wb+")


	for byte in hexDataList:
		f.write(int(byte,16).to_bytes(1, byteorder='big'))

'''
b'\xff'
b'\xd9'
b'\xab'
b'\x12'
b'\x03'
b'S'
b'\xff'
b'\xd9'
'''

	f.close()


