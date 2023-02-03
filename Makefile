CC = g++ -I ./include -c -DDEBUG
LD = g++


SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)



%.o: %.cpp
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o as
	./as -i bios.asm -o std_bios.bin
	./emulator


clean:
	rm *.o as -rf
