CC=mpic++
CFLAGS=-std=c++11 -lm  -O3 -fopenmp
LFLAGS=-lconfig++


all: model


model:	utils.cpp utils.h population.cpp population.h main.cpp area.cpp area.h const.h
	$(CC) $(CFLAGS) $(LFLAGS) main.cpp utils.cpp population.cpp area.cpp -o model

utils:	utils.cpp utils.h
	$(CC) $(CFLAGS) -DDEBUG utils.cpp -o utils

clean:
	rm -rf *~ utils model *.o

