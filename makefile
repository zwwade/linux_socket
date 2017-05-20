.SUFFIXES:.c .o

CC=gcc
SRCS=socket_test.c\
		AMSPSocket.c\
		AMSPSystem.c

OBJS=$(SRCS:.c=.o)
EXEC=socket_test


start: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS)
	@echo '----------------ok------------'

.c.o:
	$(CC) -o $@ -c $<

clean:
	rm -f $(OBJS)
