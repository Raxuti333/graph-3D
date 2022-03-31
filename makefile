CC=gcc
CFLAGS=-O2 -march=native
LFLAGS=-lglfw -lGL -lGLU -lGLEW -lm
SRC=src/*
BIN="graph3D"

build:
	@${CC} ${CFLAGS} -o ${BIN} ${SRC} ${LFLAGS}