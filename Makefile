obj-m+=led_handler.o

help:
	@echo "Commands to handle this LKM:"
	@echo -n "\t'make setup' - Sets up the scripts necessary for this LKM\n"
	@echo -n "\t'make install' - Compiles the LKM and inserts it into the kernel\n"
	@echo -n "\t'make clean' - Removes the LKM from the kernel and removes installation and extra files\n"
	@echo -n "\t'make restart' - Performs 'make clean' and 'make install' at once\n"

setup:
	chmod +x B*.sh

install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	sudo insmod led_handler.ko
clean:
	rm -f B*.log
	sudo rmmod led_handler.ko
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
restart:
	rm -f B*.log
	sudo rmmod led_handler.ko
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	sudo insmod led_handler.ko
