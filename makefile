PROGRAM=wat
MAIN=wat
WAV_FILE=delicia.wav
GDB=gdb
GDB_FLAG=-g
LOG=log
DFT=dft

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c $(LOG).c
	cc -o $(PROGRAM) $(MAIN).c $(LOG).c $(DFT).c

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LOG).c $(DFT).c $(GDB_FLAG)

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
