obj-m += kmod-test.o

all:
	CROSS_COMPILE=arm-linux- ARCH=arm make -C /home/jronen/chumby/kernel/linux-2.6.28.mx233/ M=$(PWD) modules
clean:
	mCROSS_COMPILE=arm-linux- ARCH=arm make -C /home/jronen/chumby/kernel/linux-2.6.28.mx233/ M=$(PWD) clean
