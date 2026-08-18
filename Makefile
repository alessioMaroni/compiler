
all:
	rm -rf .build
	mkdir -p .build
	gcc main.c -o .build/main.bin
	./.build/main.bin programs/program.c
