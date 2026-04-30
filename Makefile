# FOR BajeEngineChess
# Makefile created by Chandan Das

CC       = gcc
RM       = rm -f

INFILE   = bajeEngin07.c
BIN      = bajeEngin


CFLAGS   = -O3 -s
LINKS    = -lm
OPTIONS  = $(LINKS) $(CFLAGS) 

all: $(BIN)	
$(BIN): $(INFILE)
	$(CC) -o $(BIN) $(INFILE) $(OPTIONS)
	
play: $(BIN)
	./$(BIN) play


clean:
	$(RM) $(BIN)