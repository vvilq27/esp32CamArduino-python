import serial


# may need to add pic id


ser = serial.Serial('COM10', 115200, timeout=10 )#, parity=serial.PARITY_EVEN, rtscts=1)

newImg = False

# imageBytes = ''

while not newImg:
	packet = ser.readline()
	newImg = packet[:8] == b'received'
	continue

while True:
	newImg = False

	# why bytearray?
	imageBytesTab = bytearray()

	# packet = packet.decode('utf-8')
	packet = ''

	tab = []

	while not newImg:
		packet = ser.readline()
		newImg = packet[:8] == b'received'
		if not newImg:
			for b in packet[:-2]:
				print(format(b, 'x'), end='')
			print()
			tab.append(packet)
		else:
			packet = packet.decode('utf-8')
			packetParts = packet.split(' ')[4:6]
			print(packetParts)
		continue

	# ser.close()

	if len(tab) == 0:
		continue

	# //check if last array has ffd9
	# for tabPacket in tab:
	for i in range(len(tab)):

		#last packet case
		if i == len(tab) - 1:
			# s = tab[i].decode('utf-8')
			# s = s[2:-2]
			# s = s.rstrip("0")		

			# imageBytes += s 

			imageBytesTab.extend( [b for b in tab[i][1:-2]])

		#every other packet
		else :
			# imageBytes += tab[i][1:-2]
			
			# print([b for b in tab[i][1:-2]]) # [35, 156, 230, 173, 104, 86, 165, 115, 77, 170, 70, 129, 72, 121, 165, 212, 67, 105, 126, 181, 66, 176, 148, 153, 21, 42, 227, 18, 151, 21, 64]
			imageBytesTab.extend( [b for b in tab[i][1:-2]])
			# print(tab[i])
			# print([b for b in tab[i][1:-2]])
			# print(imageBytesTab[-31:])

	print(len(imageBytesTab))

	if(len(imageBytesTab) > 10000):
		continue

	print(imageBytesTab[:30])
	print(imageBytesTab[-40:])

	# remove trailing 0s
	while imageBytesTab[-1] == 0:
		del imageBytesTab[-1]

	f = open("imgPort.jpg", "wb+")

	f.write(imageBytesTab)

	f.close()


# inverse int(c, 16)
# >>> i = 0
# >>> for b in d:
# ...     if b < 16:
# ...             print('0', end ='')
# ...     print(format(b,'x'), end = '')
# ...     i += 1
# ...     if i%32 ==0:
# ...             print()