all: msh
	./msh
msh: msh.c
	gcc -g -Wall msh.c -o msh
clean:
	rm msh
