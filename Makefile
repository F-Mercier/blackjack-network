all: server client test_pseudos

server: sample_server.o threads_manager.o pseudos.o players.o
	gcc -w $^ -pthread -o server

client: sample_client.c
	gcc -w sample_client.c -o client

test_pseudos: test.c pseudos.c pseudos.h
	gcc -w $^ -o $@

sample_server.o: sample_server.c

threads_manager.o: threads_manager.c threads_manager.h

pseudos.o: pseudos.c pseudos.h

players.o: players.c players.h pseudos.h

PHONY:clean

clean:
	@rm *~ *.o ./server ./client ./test_pseudos
