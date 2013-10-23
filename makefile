PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g

CC = cc
CC_FLAGS = -Wall

LIBS += log.c
LIBS += dft.c
LIBS += benchmark.c
LIBS += fft.c
LIBS += utils.c

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c $(LIBS)
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS)

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LIBS) $(GDB_FLAG)

clean:
	$(RM) -R wat.dSYM/
	$(RM) *.out
	$(RM) $(PROGRAM)

output:
	$(RM) _*

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
