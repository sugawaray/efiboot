.POSIX:

TARGET = utest ltest loadscts_test

RM = rm

all: $(TARGET)

clean:
	-$(RM) $(TARGET) $(EOBJS) $(OBJS)

EOBJS = utest.o ltest.o loadscts_test.o
OBJS = utils.to loader.to stub.to

$(TARGET): $(EOBJS) $(OBJS)
	$(LD) -o $@ $@.o $(OBJS) $(LDFLAGS)

CC = gcc
LD = gcc

CFLAGS = -Wall -pedantic -std=c99 -fshort-wchar
CPPFLAGS = -I/usr/include/efi/ -I/usr/include/efi/x86_64/

LDFLAGS = 

.SUFFIXES: .o .to

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.to:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

utils.to: utils.c
loader.to: loader.c loader.h elf.h
ltest.o: ltest.c loader.h elf.h
loadscts_test.o: loadscts_test.c loader.h elf.h
utest.o: utest.c efiboot.h
stub.o: stub.c stub.h
