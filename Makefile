INCLUDE_DIRS = -I/usr/include -I/usr/local/include
LIB_DIRS = 
CC = gcc

CDEFS = 
CFLAGS = -O0 -g -pthread -D_GNU_SOURCE $(INCLUDE_DIRS) $(CDEFS)
LIBS = -lrt 

# List of header files
HFILES = analysis.h device_init.h global.h service_1.h service_2.h service_3.h service_4.h

# List of source files
CFILES = analysis.c device_init.c main.c service_1.c service_2.c service_3.c service_4.c

# Generate object file list from source file list
OBJS = ${CFILES:.c=.o}

# Target executable
TARGET = capture

all: $(TARGET)

clean:
	-rm -f *.o *.d *.csv
	-rm -f $(TARGET)

distclean: clean

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

depend:
	makedepend $(INCLUDE_DIRS) $(CFILES)

.c.o:
	$(CC) $(CFLAGS) -c $<

# DO NOT DELETE THIS LINE -- make depend needs it
