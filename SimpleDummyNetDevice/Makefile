# Cross compiler makefile for FIFO DMA example

KERN_SRC=/home/mike/mykernel/linux-xlnx-branch-redpitaya-v2016.2
####### KERN_SRC=/home/mike/mykernel/linux-xlnx-xilinx-v2016.4

obj-m := demin.o

all:
	make -C $(KERN_SRC) ARCH=arm M=`pwd` modules
clean:
	make -C $(KERN_SRC) ARCH=arm M=`pwd=` clean
#
#
