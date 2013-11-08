PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g

CC = cc
CC_FLAGS = -Wall

n = 1
ARG = NONE

OPT_FLAGS = -Ofast

LIBS += wat.c
LIBS += log.c
LIBS += benchmark.c
LIBS += utils.c

OBJ += fft.o

THREAD += -lpthread
THREAD += -D
THREAD += HAVE_THREADS

.PHONY:$(PROGRAM)

$(PROGRAM): $(LIBS) $(OBJ)
	$(CC) -o $(PROGRAM) $(LIBS) $(OBJ) $(CC_FLAGS) -D ORIG -D GO

$(GDB): $(LIBS) $(OBJ)
	cc -o $(PROGRAM) $(LIBS) $(OBJ) $(GDB_FLAG) -D GO

thread:
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS) $(CC_FLAGS) $(THREAD) -D GO

$(OBJ): fft.c
	$(CC) -c fft.c -o fft.o $(OPT_FLAGS)

fast:
	$(CC) -o $(PROGRAM) fft.c $(LIBS) $(CC_FLAGS) $(OPT_FLAGS) -D GO


#==============================================================================#
# OPTIMIZATIONS #

opt:
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS) $(CC_FLAGS) -D OPT  -D $(ARG)


orig:
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS) $(CC_FLAGS) -D ORIG_B -D $(ARG)


fission:
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS) $(CC_FLAGS) -D FISSION -D $(ARG)


# OPTIMIZATIONS #
#==============================================================================#

clean:
	$(RM) -R wat.dSYM/
	$(RM) *.out
	$(RM) $(PROGRAM)

output:
	$(RM) _*
