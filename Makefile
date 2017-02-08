all: server client

server: sample_server.c
	gcc -w sample_server.c -pthread -o server

client: sample_client.c
	gcc -w sample_client.c -o client
