CC = gcc
CFLAGS = -lpthread -Wformat -Wall
TARGET = server
SRCS = server.c server_client.c list.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
