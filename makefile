# Main compiler
CC=g++
SRCEXT=cpp

# All important subdirectories
# include contains all .h/.hpp files
IDIR=src

# object files
ODIR=obj

# libraries
LDIR=lib

# source files
SDIR=src

# test files
TESTDIR=tests

# Compilation flags
FLAGS=-Wall -std=c++17 -O2 -I $(IDIR) -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

# List of all sources and objects
SRC=$(shell find $(SDIR) -type f -name *.$(SRCEXT))

# Baseclasses
BASESRCDIR=$(SDIR)/baseclasses
BASESRC=$(shell find $(BASESRCDIR) -type f -name *.$(SRCEXT))
BASEOBJ=$(patsubst $(BASESRCDIR)/%.$(SRCEXT), $(ODIR)/%.o, $(BASESRC))

$(ODIR)/%.o: $(BASESRCDIR)/%.$(SRCEXT)
	@$(CC) -c -o $@ $^ $(FLAGS)

# Client
CLIENTSRC=src/client.cpp

pscplm30-client.o: $(CLIENTSRC)
	@$(CC) -c -o $@ $^ $(FLAGS)

pscplm30-client: $(BASEOBJ) pscplm30-client.o
	@$(CC) -o $@ $^ $(FLAGS)

# Server
SERVERSRC=src/server.cpp

pscplm-server.o: $(SERVERSRC)
	@$(CC) -c -o $@ $^ $(FLAGS)
	
pscplm30-server: $(BASEOBJ) pscplm30-server.o
	@$(CC) -o $@ $^ $(FLAGS)

.PHONY: clean
.PHONY: echoSymbols

clean:
	rm $(shell find $(ODIR) -type f -name *.o)

echoSymbols:
	@echo $(BASEOBJ)
