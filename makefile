PROGRAM=learning_read_bytes
MAIN=main

$(PROGRAM): $(MAIN).c
	cc -o $(PROGRAM) $(MAIN).c

run:
	./$(PROGRAM)
