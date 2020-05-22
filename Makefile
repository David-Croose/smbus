TARGET = smbus.elf
CROSS_COMPILE = arm-linux-gnueabihf-

all:
	$(CROSS_COMPILE)gcc -Wall -O0 -g main.c -o $(TARGET)

clean:
	rm -rf $(TARGET)
