PROGRAM=wat
MAIN=wat
WAV_FILE=delicia.wav
GDB=gdb
GDB_FLAG=-g
LOG=log

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c $(LOG).c
	cc -o $(PROGRAM) $(MAIN).c $(LOG).c

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LOG).c $(GDB_FLAG)

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
