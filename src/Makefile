
CC     = gcc
CFLAGS = -Wall -O2
AR     = ar
RM     = rm -f

OUTDIR = ..
TARGET = $(OUTDIR)/libminiwav.a
SRCS   = $(wildcard *.c)
OBJS   = $(addprefix $(OUTDIR)/, $(SRCS:.c=.o))

.PHONY: all clean

# commands
all: $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)
	
# dependencies
$(TARGET): $(OBJS)
	$(AR) rcs $@ $^

$(OUTDIR)/%.o: %.c
	@[ -d $(OUTDIR) ]
	$(CC) -c -o $@ $< $(CFLAGS)
