SKIN=alchemy
MAIN_SRC=main
TARGET=XenoSidexddpExample

EXEC=XenoSidexddpExample
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

LM=-lm

CFLAGS := $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --cflags)
LDFLAGS := $(LM) $(shell /usr/xenomai/bin/xeno-config --skin=alchemy --ldflags)
CC := $(shell /usr/xenomai/bin/xeno-config --cc)

all: $(EXEC)

EDFtest: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: listener.h loop_task.h
        %.o: %.c

	$(CC) -o $@ -c $< $(CFLAGS)
clean:
	rm -f *.o core

mrproper: clean
	rm -f $(EXEC)

#$(TARGET): $(MAIN_SRC).c
#	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
