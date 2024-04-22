#CC:=clang
CFLAGS+=-std=gnu99
CFLAGS+=-O2
CFLAGS+=-pedantic -Wall

LDFLAGS+=-lpthread
OBJS=threads.o

#LDFLAGS+=-lstdthreads
#OBJS=threads-c11.o

TARGET=threads

bin: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

