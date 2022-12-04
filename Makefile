obj-m += mod.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rm -f usermod
test:
	sudo dmesg -C
	sudo insmod mod.ko
	dmesg
usermod:
	gcc -pedantic-errors -Wall -Werror -g3 -O0 --std=c99 -fsanitize=address,undefined,leak ./usermod.c -o usermod

