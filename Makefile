GCC = gcc
CFLAGS = -Wall
POSIX = -lpthread

PROGRAMAS = generadorCSV agentePais servidorISE

all: $(PROGRAMAS)

generadorCSV: generadorCSV.c
	$(GCC) $(CFLAGS) $@.c -o $@

agentePais: agentePais.c
	$(GCC) $(CFLAGS) $@.c -o $@

servidorISE: servidorISE.c COMUN.h
	$(GCC) $(CFLAGS) servidorISE.c -o $@ $(POSIX)

clean:
	$(RM) $(PROGRAMAS) encuestas_*.csv PipeN ise_fifo
