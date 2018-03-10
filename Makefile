# breakout

include config.mk

SRC = breakout.c util.c
OBJ = ${SRC:.c=.o}

BIN = breakout

all: options ${BIN}

options:
	@echo breakout build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk util.h

${BIN}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f ${BIN} ${OBJ}

run: all
	./${BIN}

install: all
	@echo installing executable file to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@install -m 755 ${BIN} ${PREFIX}/bin/${BIN}

uninstall:
	@echo removing executable file from ${PREIX}/bin
	@rm -f ${PREFIX}/bin/${BIN}

.PHONY: all options clean install uninstall
