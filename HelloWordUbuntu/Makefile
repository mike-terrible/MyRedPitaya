# Cross compiler makefile for FIFO DMA example

#KERN_SRC=/home/mike/mykernel/linux-xlnx-branch-redpitaya-v2016.2
####### KERN_SRC=/home/mike/mykernel/linux-xlnx-xilinx-v2016.4

obj-m := hello.o

all:
	make -C /lib/modules/`uname -r`/build M=`pwd`
clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
#
#
