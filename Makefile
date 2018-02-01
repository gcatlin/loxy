NAME = loxy
SRC_FILES = main.c
CC_FLAGS = -g -std=c11 -Wall -Wextra \
		   -Wno-unused-function -Wno-unused-parameter
CC = clang

all:
	${CC} ${SRC_FILES} ${CC_FLAGS} -o ${NAME}
