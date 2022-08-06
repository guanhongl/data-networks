PROGRAM=client
CC=gcc

run: $(PROGRAM)
	./$(PROGRAM) 25603

$(PROGRAM): $(PROGRAM).o
	$(CC) $(PROGRAM).o -o $(PROGRAM)

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -c $(PROGRAM).c

clean:
	rm *.o $(PROGRAM)
