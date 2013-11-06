PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g

CC = cc
CC_FLAGS = -Wall
n = 1

OPT_FLAGS = -Ofast

LIBS += log.c
LIBS += dft.c
LIBS += benchmark.c
LIBS += fft.c
LIBS += utils.c

THREAD += -lpthread
THREAD += -D
THREAD += HAVE_THREADS

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c $(LIBS)
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS)

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LIBS) $(GDB_FLAG)

thread:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) $(THREAD)

fast:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) $(OPT_FLAGS) 

fission:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D FISSION -D FISSION_V1

fission2:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D FISSION -D FISSION_V2

wat_fission:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D WAT_FISSION

breno400:
	-./$(PROGRAM) -i _r_esperando.wav > _benchmark400.txt -b $(n)

breno40:
	-./$(PROGRAM) -i esperando.wav > _benchmark40.txt -b $(n)






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
