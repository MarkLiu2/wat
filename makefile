PROGRAM=wat
MAIN=wat

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c
	cc -o $(PROGRAM) $(MAIN).c

run:
	-./$(PROGRAM)
