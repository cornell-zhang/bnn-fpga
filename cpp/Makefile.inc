CXX=g++
CFLAGS=-O3 -std=gnu++11 -g
MINIZIP_LDFLAGS=-lminizip -laes -lz -lrt
LDFLAGS=$(MINIZIP_LDFLAGS)

AR=ar rcs
