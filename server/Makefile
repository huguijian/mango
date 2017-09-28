
# ========================================================================================
# Set include directories.
INCLUDEDIR = /usr/local/include

# ========================================================================================
# Compile flags
CPPFLAGS = -g
CC=gcc

# ========================================================================================
# TARGET/Source definitions
SRCDIR = .
TARGET = tpool_server
OBJS = $(TARGET).o
FILES= /thirdparty/http-parser/http_parser.c\
       mango_server.c\
       mango_connect.c\
       mango_socket.c\
       mango_log.c\

SRC = $(wildcard $(SRCDIR)/$(FILES))
# ========================================================================================
# Make all option.
LDLIBS = 
LIBS = -l pthread

all: $(TARGET)
$(TARGET):$(SRC)
		$(CC) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS) $(LIBS)

# ========================================================================================
# Clean all option

clean:
	rm -f $(SRCDIR)*.o $(SRCDIR)*~ $(SRCDIR)fcs $(SRCDIR).depend
	rm -f *.o *~ $(TARGET) .depend $(TARGET)



