PROGRAM=wat
MAIN=wat
WAV_FILE=delicia.wav
GDB=gdb
GDB_FLAG=-g

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c
	cc -o $(PROGRAM) $(MAIN).c

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(GDB_FLAG)

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
