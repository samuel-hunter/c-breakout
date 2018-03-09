# breakout
PREFIX = ${HOME}/.local

CC = clang

CFLAGS = -g -std=c99 $(shell sdl2-config --cflags) -Wpedantic -Os
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf
