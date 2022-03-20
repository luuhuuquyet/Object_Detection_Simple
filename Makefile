CC=g++

all: main.cpp
	$(CC) -o main main.cpp
clean:
	rm main *.o
