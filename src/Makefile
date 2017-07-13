TARGS=naive multproc multhread select poll epoll epoll-et

CC=gcc
CFLAGS:=$(CFLAGS) #-I./ 
LDFLAGS:=$(LDFLAGS) -lpthread #-Llib 

%: %.c
	$(CC) -o $@ $< $(LDFLAGS) $(CFLAGS)

all: $(TARGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGS)

