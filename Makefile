CC = gcc
CFLAG = -c
OFLAG = -o
MYDIZOBJS = mydiz.o
MYDIZ = ./mydiz
CREATOBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o create.o
CREAT = ./create
COMPROBJS =  compression.o stattools.o
COMPR = ./compress
APPOBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o append.o
APP = ./append
METAOBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o metadata.o
META = ./metadata
PRINTOBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o print.o
PRINT = ./print
EXPOBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o export.o
EXP = ./export
QUEROBJS = header.o mmFile.o diNode.o directoryFile.o list.o stattools.o query.o
QUER = ./query
HDRS = header.h mmFile.h diNode.h directoryFile.h list.h stattools.h diHeader.h 
SRCS = header.c mmFile.c diNode.c directoryFile.c list.c stattools.c append.c create.c compression.c mydiz.c metadata.c print.c export.c query.c
OBJS = $(SRCS: .c=.o)

all: $(MYDIZ) $(CREAT) $(APP) $(META) $(PRINT) $(EXP) $(QUER) $(COMPR)

$(CREAT) : $(CREATOBJS)
	$(CC) $(OFLAG) $@ $(CREATOBJS)

$(APP) : $(APPOBJS)
	$(CC) $(OFLAG) $@ $(APPOBJS)

$(MYDIZ) : $(MYDIZOBJS)
	$(CC) $(OFLAG) $@ $(MYDIZOBJS)

$(META) : $(METAOBJS)
	$(CC) $(OFLAG) $@ $(METAOBJS)

$(PRINT) : $(PRINTOBJS)
	$(CC) $(OFLAG) $@ $(PRINTOBJS)
	
$(EXP) : $(EXPOBJS)
	$(CC) $(OFLAG) $@ $(EXPOBJS)

$(QUER) : $(QUEROBJS)
	$(CC) $(OFLAG) $@ $(QUEROBJS)

$(COMPR) : $(COMPROBJS)
	$(CC) $(OFLAG) $@ $(COMPROBJS)
