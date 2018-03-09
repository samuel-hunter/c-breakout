# breakout

include config.mk

SRC = breakout.c
OBJ = ${SRC:.c=.o}

all: options breakout

options:
	@echo breakout build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

breakout: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f breakout ${OBJ}

install: all
	@echo installing executable file to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@install -m 755 ${PREFIX}/bin/breakout

uninstall:
	@echo removing executable file from ${PREIX}/bin
	@rm -f ${PREFIX}/bin/dwm

.PHONY: all options clean install uninstall
