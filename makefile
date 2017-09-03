#cible speciales
.PHONY: clean, cleanall
# désactivation des règles implicites
.SUFFIXES: 

# file locations: 
SRCDIR=src
INCDIR= -Iinclude

EXEDIR=bin
OBJDIR=obj

#for ROOT
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --glibs)

CC = g++  # compiler
CFLAGS = -Wall -ansi -pedantic -g -fPIC  #compiler options

_EXE = server_1
_OBJ = 

SRCS = $(patsubst %,$(SRCDIR)/%.cc,$(_OBJ))
SRCS += $(patsubst %,$(SRCDIR)/%.cc,$(_EXE))

OBJS1 = $(patsubst %,$(OBJDIR)/%.o,$(_OBJ))
OBJS2 = $(patsubst %,$(OBJDIR)/%.o,$(_EXE))

EXE  = $(patsubst %,$(EXEDIR)/%,$(_EXE))


all : $(EXE)

$(EXE): $(OBJS1) $(OBJS2)
	$(CC) -o $@  $(CFLAGS)  $(patsubst %,$(OBJDIR)/%.o,$(notdir $@)) $(OBJS1) $(LINKSEARCH) $(LIBS) $(ROOTFLAGS) $(ROOTLIBS) 

$(OBJDIR)/%.o : $(SRCDIR)/%.cc
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIR) $(ROOTFLAGS)

cleanall: clean
	rm -f *~
	rm -f *#
	rm -f ./bin/hserv 
	rm -f ./bin/hClient
	rm -f ./bin/main
	rm -f ./obj/*.o

clean:
	rm -f *~
	rm -f *#
	rm -f ./obj/*.o

