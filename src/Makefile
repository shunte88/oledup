VERSION=0.1
DESTDIR?=/usr
PREFIX?=/local

PROJ=ssd1306_i2c

LIB_A=lib$(PROJ).a
LIB_SO=lib$(PROJ).so
LIB_SO_VER=$(LIB_SO).$(VERSION)
LIB_SRCS=$(PROJ).c
LIB_OBJS=$(LIB_SRCS:.c=.o)

INSTBASE=$(DESTDIR)$(PREFIX)
BINDIR=$(INSTBASE)/bin
INCDIR=$(INSTBASE)/include
LIBDIR=$(INSTBASE)/lib

HEADERS=$(shell ls *.h)

CMD=sysinfoled
CMD_SRCS=$(CMD:=.c)
CMD_OBJS=$(CMD_SRCS:.c=.o)

INCLUDE=-I.
DEBUG=-O2
CFLAGS=$(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-L$(LIBDIR) -lwiringPi -lm -lpthread -lrt -lcrypt

all: core

core:	$(CMD)
%:	%.c $(LIBDIR)/$(LIB_SO)
	$(CC) -o $@ timer.c mem_info.c fan_control.c $^ -l$(PROJ) $(LIBS)

$(LIB_SO_VER):	$(LIB_OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_SO) -Wl,-rpath=$(LIBDIR) -o $@ $(LIBS) $^

$(LIBDIR)/$(LIB_SO):	$(LIB_SO_VER)
	install -m 0755 -d $(INCDIR)
	install -m 0644 $(HEADERS) $(INCDIR)
	install -m 0755 $(LIB_SO_VER) $(LIBDIR)
	ln -sf $^ $@
	ldconfig $(LIBDIR)

install:	$(CMD)
	install -m 0755 -d $(BINDIR)
	install -m 0755 $(CMD) $(BINDIR)

uninstall:
	-for c in $(CMD); do rm -fv $(BINDIR)/$$c; done
	-for h in $(HEADERS); do rm -v $(INCDIR)/$$h; done
	-for l in $(LIB_SO_VER) $(LIB_SO); do rm -fv $(LIBDIR)/$$l; done

clean:
	rm -f *.o $(CMD) $(LIB_SO) $(LIB_SO_VER) $(BINDIR)/$(CMD)
