CC=gcc
CFLAGS=-Wall -Werror
LIBS=-ludis86
NAME=accessstring
INSTALL_PATH=/usr/bin/$(NAME)

all: $(NAME)

$(NAME): AccessString.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:
	cp $(NAME) $(INSTALL_PATH)

uninstall:
	rm $(INSTALL_PATH)

clean:
	rm -f $(NAME)
