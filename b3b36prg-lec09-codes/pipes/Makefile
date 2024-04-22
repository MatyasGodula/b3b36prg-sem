CFLAGS+= -Wall -Werror -std=gnu99 -g -pedantic
LDFLAGS=-pthread

BINARIES=prg-lec09-module prg-lec09-main


all: $(BINARIES)

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJ_IO=prg_io_nonblock.o

prg-lec09-module : prg-lec09-module.o $(OBJ_IO) 
	$(CC) $< $(OBJ_IO) $(LDFLAGS) -o $@

prg-lec09-main : prg-lec09-main.o $(OBJ_IO) 
	$(CC) $< $(OBJ_IO) $(LDFLAGS) -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)
