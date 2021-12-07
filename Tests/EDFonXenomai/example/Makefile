SKIN=alchemy
MAIN_SRC=cyclic_test
TARGET=cyclic_test

LM=-lm

CFLAGS := $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --cflags)
LDFLAGS := $(LM) $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --ldflags)
CC := $(shell /usr/xenomai/bin/xeno-config --cc)

$(TARGET): $(MAIN_SRC).c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
