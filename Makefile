# FOR BajeEngineChess
# Makefile created by Chandan Das

CC       = gcc
RM       = rm -f

INFILE   = bajeEngin07.c
BIN      = bajeEngin


CFLAGS   = -Wall
LINKS    = -lm
OPTIONS  = $(LINKS) $(CFLAGS) 

all: $(BIN)
	./$(BIN)
	
$(BIN): $(INFILE)
	$(CC) -o $(BIN) $(INFILE) $(OPTIONS)

clean:
	$(RM) $(BIN)