CC=gcc
CFLAGS=-O2 -march=native
LFLAGS=-lglfw -lGL -lGLU -lGLEW -lm
SRC=src/*
BIN="3D math"

build:
	@${CC} ${CFLAGS} -o ${BIN} ${SRC} ${LFLAGS}