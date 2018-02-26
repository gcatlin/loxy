NAME = loxy
SRC_FILES = main.c
CC_FLAGS = -g -std=c11 -Wall -Wextra -Wpedantic \
		   -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers \
		   -fsanitize=address
CC = clang

all: build

build:
	@${CC} ${SRC_FILES} ${CC_FLAGS} -o ${NAME}

run: build
	@./${NAME}
