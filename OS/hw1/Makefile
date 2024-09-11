CC=gcc
all : Simple_Shell
Simple_Shell: Simple_Shell.o
	$(CC) -o Simple_Shell Simple_Shell.o 
	
Simple_Shell.o: Simple_Shell.c
	$(CC) -c -o Simple_Shell.o Simple_Shell.c
	
clean:
	rm -f Simple_Shell.*o
