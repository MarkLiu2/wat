PROGRAM = wat
MAIN = wat
GDB = gdb
GDB_FLAG = -g

CC = cc
CC_FLAGS = -Wall

n = 1
ARG = GO

OPT_FLAGS = -Ofast

MAIN_LIB += wat.c

LIB += wat.c
LIB += log.c
LIB += benchmark.c
LIB += utils.c

OBJ += wat.o
OBJ += log.o
OBJ += benchmark.o
OBJ += utils.o

OPT_OBJ += fft.o
OPT_LIB += fft.c

THREAD += -lpthread
THREAD += -D
THREAD += HAVE_THREADS

.PHONY:$(PROGRAM)

$(PROGRAM): $(LIB) $(OPT_OBJ) 
	$(CC) -o $(PROGRAM) $(LIB) $(OPT_OBJ) $(CC_FLAGS) -D ORIG -D GO

$(GDB): $(LIB) $(OPT_OBJ)
	cc -o $(PROGRAM) $(LIB) $(OPT_OBJ) $(GDB_FLAG) -D GO

thread: $(LIB) $(OPT_OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIB) $(CC_FLAGS) $(THREAD) -D GO

$(OPT_OBJ): $(OPT_LIB)
	$(CC) -c $(OPT_LIB) -o $(OPT_OBJ) $(OPT_FLAGS) -D ORIG


#==============================================================================#
# OPTIMIZATIONS #

opt: $(LIB) $(OPT_OBJ) 
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D OPT  -D $(ARG)

unroll: $(LIB) $(OPT_OBJ)  
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D UNROLL -D $(ARG)

unrofis: $(LIB) $(OPT_OBJ)  
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D UNROFIS -D $(ARG)

orig: $(LIB) $(OPT_OBJ)  
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D ORIG_B -D $(ARG)


fission: $(LIB) $(OPT_OBJ)  
	$(CC) -o $(PROGRAM) $(LIB) $(OPT_OBJ) $(CC_FLAGS) -D FISSION -D $(ARG)


# OPTIMIZATIONS #
#==============================================================================#

clean:
	$(RM) -R wat.dSYM/
	$(RM) *.out
	$(RM) $(PROGRAM)
	$(RM) *.o

output:
	$(RM) _*
