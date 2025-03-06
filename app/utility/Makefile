all : ../lib/can.o

CC=/home/teuto/Documents/10.SEC/1.PSE/tools-master/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc


../lib/can.o: src/can.c
	$(CC) -c $^ -o $@

clean:
	rm -rf ../lib/*