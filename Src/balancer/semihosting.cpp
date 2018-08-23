#include <cstring>

//#define SEMIHOSTING_ENABLE

#ifdef SEMIHOSTING_ENABLE

#define SYS_OPEN 0x01
#define SYS_WRITEC 0x03
#define SYS_WRITE 0x05

extern "C" {
	unsigned int semihosting(int sys, void *args) {
		unsigned int result;
		asm (
		"mov r0, %[sys]\n"
		"mov r1, %[args]\n"
		"bkpt #0xAB\n"
		"mov %[result], r0\n"
		: [result] "=r"(result)
		: [args] "r"(args), [sys] "r"(sys)
		: "r0", "r1"
		);
		return result;
	}

	void put_char(char c) {
		semihosting(SYS_WRITEC, &c);
	}

	int _open(char* file, int flags, int mode) {
		int args[] = {(int) file, 7 /* w+ */ , (int) strlen(file)};
		return semihosting(SYS_OPEN, args);
	}

	int _write(int file, char *data, int len) {
		int args[] = {file, (int) data, len};
		return semihosting(SYS_WRITE, args);
	}
};

#endif