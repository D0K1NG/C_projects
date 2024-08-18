BINARY = cube
INCDIR = ./Inc
SRCDIR = ./Src

CC = gcc
OPT = -O0
<<<<<<< HEAD
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(foreach D, $(INCDIR), -I$(D)) $(OPT) $(DEPFLAGS)

CFILES = $(foreach D, $(SRCDIR), $(wildcard $(D)/*.c))
OBJFILES = $(patsubst %.c, %.o, $(CFILES))
DEPFILES = $(patsubst %.c, %.d, $(CFILES)) 
=======
CFLAGS = -Wall -Wextra -g $(foreach D, $(INCDIR), -I$(D)) $(OPT)

CFILES = $(foreach D, $(SRCDIR), $(wildcard $(D)/*.c))
OBJFILES = $(patsubst %.c, %.o, $(CFILES))
>>>>>>> d05f865 (Rotating cube program v1.0)

all: $(BINARY)

$(BINARY): $(OBJFILES)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
<<<<<<< HEAD
	rm -f $(OBJFILES) $(BINARY) $(DEPFILES) *.o
=======
	rm -f $(OBJFILES) $(BINARY) *.o
>>>>>>> d05f865 (Rotating cube program v1.0)

-include $(DEPFILES)