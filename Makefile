GCC = gcc
POSIX = -lpthread

PROGRAMAS = generadorCSV agentePais

ALL: $(PROGRAMAS)

generadorCSV: generadorCSV.c
	$(GCC) $(CFLAGS) $@.c -o $@

agentePais: agentePais.c
	$(GCC) $(CFLAGS) $@.c -o $@

clear:
	$(RM) $(PROGRAMAS) encuestas_*.csv
