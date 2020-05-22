#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#if 0
this is some code reference from linux

#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		// SMBus 2.0
#define I2C_SMBUS_I2C_BLOCK_DATA    8

union i2c_smbus_data {
	__u8 byte;
	__u16 word;
	__u8 block[I2C_SMBUS_BLOCK_MAX + 2]; /* block[0] is used for length */
			       /* and one more for user-space compatibility */
};
#endif

static void print_help(void)
{
	 printf("usage: ./thisporg <device> <i2caddr> <r|w> <cmd> <size> "
			 "[data...]\n");
	 printf("example:\n");
	 printf("    /thisporg /dev/i2c-0 0x50 r 0xAA 2\n");
	 printf("    /thisporg /dev/i2c-0 0x50 w 0xBB 4 0102CDEF\n");
}

/*
 * read and write smbus
 */
int main(int argc, char *argv[])
{
	int fd;
	char *name;
	struct i2c_smbus_ioctl_data smbus_rdwr_data;
	int ret;
	char rw;
	unsigned int addr;
	unsigned char cmd;
	unsigned int size;
	unsigned int _size;
	char tmp[3];
	union i2c_smbus_data data;
	int i;

	/*
	 * check parameter
	 * TODO  there is more check work to do
	 */
	if (argc == 1) {
		print_help();
		return 0;
	}

	/*
	 * read parameter
	 */
	name = argv[1];
	addr = strtoul(argv[2], NULL, 16) & 0x7F;
	rw = argv[3][0] == 'r' ? 'r' : 'w';
	cmd = strtoul(argv[4], NULL, 16);
	size = strtoul(argv[5], NULL, 10);
	if (size > sizeof(data))
		size = sizeof(data);
	switch (size) {
	case 1:
		_size = I2C_SMBUS_BYTE_DATA;
		break;
	case 2:
		_size = I2C_SMBUS_WORD_DATA;
		break;
	default:
		_size = I2C_SMBUS_I2C_BLOCK_DATA;
	}

	printf("your enter is:\n");
	printf("    device:  %s\n", name);
	printf("    i2caddr: %#x\n", addr);
	printf("    rw:      %c\n", rw);
	printf("    cmd:     %#x\n", cmd);
	printf("    size:    %d\n", size);
	if (rw == 'w') {
		printf("    data:    ");
		tmp[sizeof(tmp) - 1] = 0;
		for (i = 0; i < size; i++) {
			tmp[0] = argv[6][i];
			tmp[1] = argv[6][i + 1];
			data.block[i] = strtoul(tmp, NULL, 16);
			printf("%02x ", data.block[i]);
		}
		printf("\n");
	}

	/*
	 * process
	 */
	fd = open(name, O_RDWR);
	if (fd) {
		printf("error: open %s failed. ret=%d\n", name, fd);
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, addr);	/* set destination address */
	if (ret < 0) {
		printf("error: ioctl(I2C_SLAVE_FORCE) with %s failed. ret=%d\n",
				name, ret);
		goto end;
	}

	smbus_rdwr_data.read_write = rw == 'r';
	smbus_rdwr_data.command = cmd;
	smbus_rdwr_data.size = _size;
	smbus_rdwr_data.data = &data;
	ret = ioctl(fd, I2C_SMBUS, &smbus_rdwr_data);
	if (ret < 0)
		printf("error: ioctl(I2C_SMBUS) with %s failed. ret=%d\n", name, ret);
	else {
		if (rw == 'r') {
			printf("read: ");
			for (i = 0; i < size; i++) {
				printf("%02x ", data.block[i]);
			}
			printf("\n");
		}
		printf("done\n");
	}

end:
	close(fd);
	return ret;
}
