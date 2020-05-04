obj-m += discosModuleko.o
discosModuleko-objs := discos.o discosModule.o

all: clean compile

compile:
	make -C /usr/src/linux SUBDIRS=$(PWD) modules
	insmod discosModuleko.ko

clean:
	-rmmod discosModuleko
	-make -C /usr/src/linux SUBDIRS=$(PWD) clean

