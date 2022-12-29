all:
	g++ as.cpp -o as -I ./include -D DEBUG
	./as std_bios.asm
	./emulator -b a.out
