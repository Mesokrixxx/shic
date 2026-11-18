NAME = libshic.a

CC = clang
CFLAGS = -std=c23 -Wall -Wextra -Iinclude/
LDFLAGS = 

PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

SRC += src/container/dynlist.c src/container/string.c
SRC += src/debug/log.c
SRC += src/error/error.c
SRC += src/io/file.c src/io/print.c 
SRC += src/math/util.c
SRC += src/memory/allocator.c src/memory/mallocator.c
SRC += src/types/any.c src/types/stringview.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	rm -f $(NAME)
	ar -crs $(NAME) $(OBJ)

install: all
	install -d "$(LIBDIR)"
	install -m 0644 "$(NAME)" "$(LIBDIR)/$(NAME)"

	rm -rf "$(INCLUDEDIR)/shic"
	install -d "$(INCLUDEDIR)/shic"

	@cd include && for header in $$(find shic -type f -name '*.h'); do \
		dir=$$(dirname "$(INCLUDEDIR)/$$header"); \
		install -d "$$dir"; \
		install -m 0644 "$$header" "$(INCLUDEDIR)/$$header"; \
	done

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all install clean fclean re
