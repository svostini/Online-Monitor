#cible speciales
.PHONY: clean, cleanall
# désactivation des règles implicites
.SUFFIXES: 

# file locations: 
SRCDIR=src
INCDIR= -Iinclude

EXEDIR=bin
OBJDIR=obj

EXTLIB+=-lstdc++
EXTLIBPATH+=
EXTLIBHEADER+=


#for ROOT
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --libs)

CC = gcc  # compiler
CFLAGS = -Wall -ansi -pedantic -g -fPIC  #compiler options

#remeber to try to use make -n to debug

_EXE = client
_OBJ = Packet32  TDCPixTimeStampCollection TDCPixTimeStamp RG_Utilities

SRCS = $(patsubst %,$(SRCDIR)/%.cc,$(_OBJ))
SRCS += $(patsubst %,$(SRCDIR)/%.cc,$(_EXE))

OBJS1 = $(patsubst %,$(OBJDIR)/%.o,$(_OBJ))
OBJS2 = $(patsubst %,$(OBJDIR)/%.o,$(_EXE))

EXE  = $(patsubst %,$(EXEDIR)/%,$(_EXE))


all : $(EXE)

$(EXE): $(OBJS1) $(OBJS2)
	$(CC) -o $@ $(CFLAGS)  $(patsubst %,$(OBJDIR)/%.o,$(notdir $@)) $(OBJS1) $(ROOTFLAGS) $(ROOTLIBS) $(EXTLIBPATH)  $(EXTLIBHEADER)  $(EXTLIB)

$(OBJDIR)/%.o : $(SRCDIR)/%.cc
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIR) $(ROOTFLAGS)  $(EXTLIBHEADER) 

cleanall: clean
	rm -f *~
	rm -f *#
	rm -f ./bin/main
	rm -f ./obj/*.o

clean:
	rm -f *~
	rm -f *#
	rm -f ./obj/*.o

