CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Wno-attributes
CPPFLAGS = -include sys/types.h -include stddef.h -include shim.h

.PHONY: all clean

all: teste

SOURCES = aes.c cipher.c entropy.c hmac.c kdf.c sha2.c main.c
HEADERS = aes.h cipher.h entropy.h hmac.h kdf.h sha2.h shim.h

teste: $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f teste
