CC = gcc
SRC = frequency_wave.c pipe_buffer.c util.c render.c fft.c
FLAGS = -Wall -Wextra -std=gnu99 -pedantic -O2 -fdiagnostics-color=auto
LIBS = -lGL -lGLU -lglut -lfftw3 -lm
EXEC = frequency_wave

#GCOV_FLAGS = -fprofile-arcs -ftest-coverage
#GPROF_FLAGS = -pg

all:
	etags --declarations *.c *.h
	$(CC) $(FLAGS) $(GCOV_FLAGS) $(SRC) -o $(EXEC) $(LIBS) $(GPROF_FLAGS)

clean:
	rm -rf $(EXEC) *~
