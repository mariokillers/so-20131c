CC = gcc
LDFLAGS = -shared
LIBDIR = .
LIBRARY = libso-commons-library.so
SOURCES = bitarray.c config.c process.c txt.c log.c temporal.c string.c error.c
OBJECTS = $(SOURCES:.c=.o)

# Objetivo global
all: $(LIBRARY) clean

# Archivos objeto a partir de .c
%.o: $(LIBDIR)/%.c
	$(CC) -c -fpic $(CFLAGS) $< -o $@

# .so a partir de archivos objeto
$(LIBRARY): $(OBJECTS)
	 $(CC) -g $(LDFLAGS) $(LIBDIR)/collections/*.c $(LIBDIR)/Connections/*.c $(OBJECTS) -o ../$(LIBRARY)

# Limpiar archivos objeto innecesarios
clean:
	rm $(OBJECTS)
