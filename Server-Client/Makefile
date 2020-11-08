CC = g++
CFLAGS = -pthread
all: client server
server :server.cc
	$(CC) $(CFLAGS) server.cc -o server
client :client.cc
	$(CC) $(CFLAGS) client.cc -o client
clean:
	rm -rf client
	rm -rf server
copy:
	cp server ./Server/server
	cp client ./h1/client
	cp client ./h2/client
	cp client ./h3/client
	cp client ./h4/client
	cp client ./h5/client
	cp client ./h6/client	
