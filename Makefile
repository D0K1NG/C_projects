BINARY = cube
INCDIR = ./Inc
SRCDIR = ./Src

CC = gcc
OPT = -O0
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(foreach D, $(INCDIR), -I$(D)) $(OPT) $(DEPFLAGS)

CFILES = $(foreach D, $(SRCDIR), $(wildcard $(D)/*.c))
OBJFILES = $(patsubst %.c, %.o, $(CFILES))
DEPFILES = $(patsubst %.c, %.d, $(CFILES)) 

all: $(BINARY)

$(BINARY): $(OBJFILES)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(BINARY) $(DEPFILES) *.o

-include $(DEPFILES)