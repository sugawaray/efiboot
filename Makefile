.POSIX:

TARGET = efiboot.efi
TARGETDBG = dbg$(TARGET)
TARGETMTEST = mtest.efi
TARGETELF = test.elf

all: $(TARGET) $(TARGETMTEST) $(TARGETELF)
	$(MAKE) -f test.mk

clean:
	$(MAKE) -f test.mk clean
	-rm $(TARGET) $(TARGETMTEST) $(CLEANOBJS) $(SOFILE) $(SOFILEMTEST) $(TARGETDBG)

CFLAGS = -gdwarf-2 -fshort-wchar -fpic -Wall -m32 -mno-mmx -mno-sse -fno-strict-aliasing -fno-merge-constants -ffreestanding -fno-stack-protector -fno-stack-check

EFIINC = /usr/include/efi
CPPFLAGS = -I$(EFIINC)/ia32 -I$(EFIINC)/protocol -I$(EFIINC) -DPHYADDR

LDFLAGS = --warn-common --no-undefined --fatal-warnings -nostdlib -znocombreloc -T /usr/lib32/elf_ia32_efi.lds -shared -Bsymbolic /usr/lib32/crt0-efi-ia32.o -L/usr/lib32 -lefi -lgnuefi /usr/lib/gcc/x86_64-linux-gnu/4.9/32/libgcc.a

OBJS = entry.o utils.o context.o loader.o efi.o
CLEANOBJS = $(OBJS) loader.o mtest.o
SOFILE = efiboot.so
SOFILEMTEST = mtest.so

$(TARGETELF): testelf.c
	$(CC) -o $@ $< -Wall -m32 -nostdlib -e main

$(TARGET): $(SOFILE)
	./so2efi $@ $(SOFILE)

$(TARGETMTEST): $(SOFILEMTEST)
	./so2efi $@ $(SOFILEMTEST)

.SUFFIXES: .o

$(SOFILE): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(SOFILEMTEST): mtest.o
	$(LD) -o $@ mtest.o $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(CPPFLAGS) $<

entry.o: entry.c
loader.o: loader.c loader.h
mtest.o: mtest.c
utils.o: utils.c
context.o: context.c
