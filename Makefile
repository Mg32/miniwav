
CC = gcc
CFLAGS = -Wall -O2

TARGET = w.exe
SRCS = miniwav.c test.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

clean: $(TARGET) $(OBJS)
	$(RM) $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) -c $< $(CFLAGS)
