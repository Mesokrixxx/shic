NAME = shic

CC = clang
CFLAGS = -std=c23 -Wall -Wextra -Iinclude/ -g3
LDFLAGS = -g

SRC += src/container/dynlist.c src/container/string.c src/container/sparseset.c src/container/bitfield.c src/container/map.c
SRC += src/debug/log.c
SRC += src/error/error.c
SRC += src/io/file.c src/io/print.c 
SRC += src/math/util.c
SRC += src/memory/allocator.c src/memory/mallocator.c
SRC += src/types/any.c src/types/stringview.c src/types/hash.c

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS
	BUILD = llvm-ar -crs
	PREFIX ?= C:/Dev/Libs
	FixPath = $(subst /,\,$1)
	DEL = del /Q /F $(call FixPath,$1) 2>nul || exit 0
	DELDIR = rmdir /S /Q $(call FixPath,$1) 2>nul || exit 0
	MKDIR = if not exist $(call FixPath,$1) mkdir $(call FixPath,$1)
	COPY = copy /Y $(call FixPath,$1) $(call FixPath,$2) >nul
else
	UNAME_S = $(shell uname -s)
	BUILD = ar -crs
	PREFIX ?= /usr/local
	DEL = rm -f $1
	DELDIR = rm -rf $1
	MKDIR = mkdir -p $1
	COPY = cp -f $1 $2
	ifeq ($(UNAME_S),Linux)
		CFLAGS += -DLINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		CFLAGS += -DMACOS
	endif
endif

LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

ifeq ($(OS),Windows_NT)
	TARGET = $(NAME).lib
else
	TARGET = lib$(NAME).a
endif

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(call DEL,$(TARGET))
	$(BUILD) $(TARGET) $(OBJ)

install: all
	$(call MKDIR,$(LIBDIR))
	$(call COPY,$(TARGET),$(LIBDIR)/$(TARGET))
	$(call DELDIR,$(INCLUDEDIR)/$(NAME))
	$(call MKDIR,$(INCLUDEDIR)/$(NAME))
ifeq ($(OS),Windows_NT)
	xcopy /E /I /Y /Q $(call FixPath,include/$(NAME)) $(call FixPath,$(INCLUDEDIR)/$(NAME)) >nul
else
	cp -R include/$(NAME)/. "$(INCLUDEDIR)/$(NAME)/"
endif

clean:
	$(call DEL,$(OBJ))

fclean: clean
	$(call DEL,$(TARGET))

re: fclean all

.PHONY: all install clean fclean re
