CC = gcc
CFLAGS = -std=c99 -g -Wall -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
OBJS = Server.o tftpTools.o
RM = rm -rf

Server: $(OBJS)
		$(CCLINK) -o Server $(OBJS)
	
tftpTools.o: tftpTools.c tftpTools.h
Server.o: Server.c Server.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* Server