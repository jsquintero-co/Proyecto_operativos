CC = gcc
CFLAGS = -Wall

all: generadorCSV

generadorCSV: generadorCSV.c
	$(CC) $(CFLAGS) -o generadorCSV generadorCSV.c

clean:
	rm -f generadorCSV encuestas_*.csv
