tst: main.o serialib.o
	g++ -o tst main.o serialib.o

main.o: main.cpp serialib.h
	g++ -c main.cpp

serialib.o: serialib.cpp serialib.h
	g++ -c serialib.cpp