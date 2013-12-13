PROGRAM = wat
MAIN = wat

CC = cc
CC_FLAGS = -Wall

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

.PHONY: $(PROGRAM)
.PHONY: $(OPT_OBJ)

$(PROGRAM): $(LIB) $(OPT_OBJ) 
	$(CC) -o $(PROGRAM) $(LIB) $(OPT_OBJ) $(CC_FLAGS) -D ORIG

thread: $(LIB) $(OPT_OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIB) $(CC_FLAGS) $(THREAD) -D ORIG 

$(OPT_OBJ): $(OPT_LIB)
	$(CC) -c $(OPT_LIB) -o $(OPT_OBJ) $(OPT_FLAGS) -D ORIG


#==============================================================================#
# OPTIMIZATIONS #

opt: $(LIB) $(OPT_OBJ) 
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D OPT

orig: $(LIB) $(OPT_OBJ)  
	$(CC) -o $(PROGRAM) $(OPT_OBJ) $(LIB) $(CC_FLAGS) -D ORIG

# OPTIMIZATIONS #
#==============================================================================#

clean:
	$(RM) -R wat.dSYM/
	$(RM) *.out
	$(RM) $(PROGRAM)
	$(RM) *.o

output:
	$(RM) _*
