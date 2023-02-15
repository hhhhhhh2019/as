CC = gcc -I ./include -c -DDEBUG -fsanitize=address -g
LD = gcc -fsanitize=address -g


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o as
	./as bios.asm -o std_bios.o
	./ld std_bios.o -o std_bios.elf
	./objdump std_bios.elf -o std_bios -c
	./emulator


clean:
	rm *.o as -rf
