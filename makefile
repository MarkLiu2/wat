PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g

CC = cc
CC_FLAGS = -Wall

n = 1
ARG = FOO

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
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D ORIG -D GO

$(GDB): $(MAIN.c)
	cc -o $(PROGRAM) $(MAIN).c $(LIBS) $(GDB_FLAG) -D $(ARG)

thread:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) $(THREAD) -D $(ARG)

fast:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) $(OPT_FLAGS)  -D $(ARG)

fission:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D FISSION -D FISSION_V1 -D $(ARG)

fission2:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D FISSION -D FISSION_V2 -D $(ARG)

wat_fission:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D WAT_FISSION -D $(ARG)

breno400:
	-./$(PROGRAM) -i _r_esperando.wav > _benchmark400.txt -b $(n) -D $(ARG)

breno40:
	-./$(PROGRAM) -i esperando.wav > _benchmark40.txt -b $(n) -D $(ARG)


opt:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D OPT  -D $(ARG)


orig:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D ORIG_B -D $(ARG)


r_fission:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D R_FISSION -D $(ARG)


strip:
	$(CC) -o $(PROGRAM) $(MAIN).c $(LIBS) $(CC_FLAGS) -D STRIP -D $(ARG)








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
