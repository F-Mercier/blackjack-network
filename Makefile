all: server client test_pseudos

server: sample_server.c
	gcc -w sample_server.c -pthread -o server

client: sample_client.c
	gcc -w sample_client.c -o client

test_pseudos: test.c pseudos.c pseudos.h
	gcc -w $^ -o $@

PHONY:clean

clean:
	@rm *~ ./server ./client ./test_pseudos
