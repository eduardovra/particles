CC=gcc
CFLAGS=-Wall -O2 -D_GNU_SOURCE -DPLATFORM_UNIX -fPIC -Werror -Wfatal-errors
LDFLAGS=`sdl-config --static-libs` -lrt
INCLUDE=`sdl-config --cflags`
SRCS=main.c
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.dep)
TARGET=particles

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<

$(OBJS): %.o: %.c %.dep
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(DEPS): %.dep: %.c Makefile
	$(CC) $(CFLAGS) $(INCLUDE) -MM $< > $@

clean:
	-rm -f *~ *.o $(TARGET)
