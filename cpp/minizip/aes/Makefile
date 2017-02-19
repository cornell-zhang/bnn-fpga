CC=cc
CFLAGS=-O -DHAVE_AES
OBJS=aescrypt.o aeskey.o aestab.o entropy.o fileenc.o hmac.o prng.o pwd2key.o sha1.o
ARFLAGS=rv
RANLIB=ranlib

.c.o:
	$(CC) -c $(CFLAGS) $*.c

libaes.a: $(OBJS)
	$(ECHO) $(AR) $(ARFLAGS) ../libaes.a $?
	$(AR) $(ARFLAGS) ../libaes.a $?
	$(RANLIB) ../libaes.a

all: libaes.a

.PHONY: clean

clean:
	rm -f *.o *.a
