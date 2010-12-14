TARGET = utoc
INST_DIR = /usr/local/bin/

CC = gcc
CCFLAG = -Wall

SRC = main.c bookmark.c
OBJ = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $<

install:
	install -m0655 -oroot -groot $(TARGET) $(INST_DIR)/
uninstall:
	rm -f $(INST_DIR)/$(TARGET)

clean:
	rm *.o
	rm $(TARGET)
