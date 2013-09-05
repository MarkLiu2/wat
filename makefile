PROGRAM=learning_read_bytes
MAIN=main

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c
	cc -o $(PROGRAM) $(MAIN).c

run:
	-./learning_read_bytes
