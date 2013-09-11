PROGRAM=wat
MAIN=wat
WAV_FILE=toque.wav

.PHONY:$(PROGRAM)

$(PROGRAM): $(MAIN).c
	cc -o $(PROGRAM) $(MAIN).c

run:
	-./$(PROGRAM) $(ARG)

arg:
	-./$(PROGRAM) $(WAV_FILE)
