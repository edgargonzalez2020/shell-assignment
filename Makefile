all: msh
	./msh
msh: msh.c
	gcc -Wall msh.c -o msh
clean:
	rm msh
