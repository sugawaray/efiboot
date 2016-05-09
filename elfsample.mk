.POSIX:

TARGET = elfsample

RM = rm

all: $(TARGET)

clean:
	-$(RM) $(TARGET)

CFLAGS = -g -Wall -pedantic -std=c99 -m32

$(TARGET): elfsample.c
