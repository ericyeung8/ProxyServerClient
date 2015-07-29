COMPILER=gcc
CFLAGS=-Wall
all:echotcp
echotcp: server client
server:  
	$(COMPILER) -o server ServerMain.c -Wall

client: 
	$(COMPILER) -o client ClientMain.c -Wall


clean:
	rm server client



