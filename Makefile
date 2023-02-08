CC = gcc -I ./include -c -DDEBUG -fsanitize=address -g
LD = gcc -fsanitize=address -g


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o as
	./as bios.asm -o std_bios.elf
	#./emulator


clean:
	rm *.o as -rf
