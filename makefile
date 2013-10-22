PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g
LIBS += log.c
LIBS += dft.c
LIBS += benchmark.c

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c $(LIBS)
	cc -o $(PROGRAM) $(MAIN).c $(LIBS)

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LIBS) $(GDB_FLAG)

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
