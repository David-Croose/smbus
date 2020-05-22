TARGEt = smbus.elf
CROSS_COMPILE =

all:
	$(CROSS_COMPILE)gcc -Wall -O0 -g main.c -o $(TARGEt)

clean:
	rm -rf $(TARGEt)
