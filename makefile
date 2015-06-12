run: main.c
	mpicc -O2 -o matvec matvecres.c genmatvec.c main.c -lm -std=c99

