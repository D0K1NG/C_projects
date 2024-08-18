BINARY = cube
INCDIR = ./Inc
SRCDIR = ./Src

CC = gcc
OPT = -O0
CFLAGS = -Wall -Wextra -g $(foreach D, $(INCDIR), -I$(D)) $(OPT)

CFILES = $(foreach D, $(SRCDIR), $(wildcard $(D)/*.c))
OBJFILES = $(patsubst %.c, %.o, $(CFILES))

all: $(BINARY)

$(BINARY): $(OBJFILES)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(BINARY) *.o

-include $(DEPFILES)