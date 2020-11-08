CC = g++
CFLAGS = -pthread
all: peer_downloader peer_uploader
peer_downloader:peer_downloader.cpp
	$(CC) $(CFLAGS) peer_downloader.cpp -o pd
peer_uploader:peer_uploader.cpp
	$(CC) $(CFLAGS) peer_uploader.cpp -o pu
clean:
	rm -rf pu pd
copy:
	cp pd ./test1/pd 
	cp pd ./test2/pd
	cp pu ./test1/pu
	cp pu ./test2/pu
