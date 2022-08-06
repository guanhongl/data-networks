PROGRAM=ecdns
CC=gcc

run: $(PROGRAM)
	./$(PROGRAM) 25603 ics651.uhm/1.2.3.4 www.ics651.uhm/4.5.6.7

$(PROGRAM): $(PROGRAM).o
	$(CC) $(PROGRAM).o -o $(PROGRAM)

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -c $(PROGRAM).c

clean:
	rm *.o $(PROGRAM)
