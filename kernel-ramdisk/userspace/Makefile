all: clean compile

run:
	./test_file

compile:
	$(CC) -m32 -Wall -I../kernelspace -o test_file test_file.c userspace_interface.c

clean:
	-rm ./test_file