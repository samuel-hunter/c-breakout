# breakout
PREFIX = ${HOME}/.local

CC = clang

CFLAGS = -g -std=c99 -D_DEFAULT_SOURCE $(shell sdl2-config --cflags) -Wpedantic -Os
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf -lm
