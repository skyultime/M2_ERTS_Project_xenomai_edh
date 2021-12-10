SKIN=alchemy
MAIN_SRC=main
TARGET=xddpEchoExample

LM=-lm

CFLAGS := $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --cflags)
LDFLAGS := $(LM) $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --ldflags)
CC := $(shell /usr/xenomai/bin/xeno-config --cc) 

$(TARGET): $(MAIN_SRC).c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
