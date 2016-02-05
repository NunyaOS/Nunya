all: iso

iso: iso.c iso.h 
	gcc -o iso iso.c 

console.o: console.c console.h
	gcc -o console.o console.c

list.o: list.c list.h
	gcc -o list.o list.c

interrupt.o: interrupt.c interrupt.h
	gcc -o interrupt.o interrupt.c

process.o: process.h process.c
	gcc -o process.o process.c

pagetable.o: pagetable.c pagetable.h
	gcc -o pagetable.o pagetable.c

iso.o: iso.c iso.h
	gcc -o iso.o iso.c

clean:
	rm -f iso *~ *.o
