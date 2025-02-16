CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -ljson-c -lpthread

TARGET = client
SRCS = client.c base64_c.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) 