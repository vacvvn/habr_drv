
# Comment/uncomment the following line to disable/enable debugging
#DEBUG = y
CC=gcc



ifneq ($(KERNELRELEASE),)
# call from kernel build system


obj-m	:= habr_drv.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.order *.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

test:
	sudo dmesg -C
	sudo insmod habr_drv.ko
	sudo rmmod  habr_drv.ko
	dmesg
 
ifeq (.depend,$(wildcard .depend))
include .depend
endif
