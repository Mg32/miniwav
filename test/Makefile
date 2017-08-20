
CC     = gcc
CFLAGS = -Wall -O2
RM     = rm -f

OUTDIR = ..
TARGET = ./test
TEST_GENERATED = ./output/*

SRCS   = $(wildcard *.c)
OBJS   = $(addprefix $(OUTDIR)/, $(SRCS:.c=.o))
INCS   = -I../src
LIBS   = -L$(OUTDIR)/ -lminiwav

.PHONY: all run clean

# commands
all: $(TARGET)

run: $(TARGET)
	$(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET) $(TEST_GENERATED)

# dependencies
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(OUTDIR)/%.o: %.c
	@[ -d $(OUTDIR) ]
	$(CC) -c -o $@ $< $(INCS) $(CFLAGS)
