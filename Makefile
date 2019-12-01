CC ?= gcc
CXX ?= g++

_FLAGS    = -Wall -Wextra -O3 -fPIC -Wno-unused-parameter
CFLAGS   += $(_FLAGS) -std=c99
CXXFLAGS += $(_FLAGS) -std=c++11

NAME = setbfree-controller

all: build
build: $(NAME).so

$(NAME).so: $(NAME).c.o
	$(CC) $^ $(LDFLAGS) -shared -Wl,--no-undefined -o $@

$(NAME).c.o: $(NAME).c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f *.o *.so

install: build
	install -d $(DESTDIR)$(PREFIX)/lib/lv2/$(NAME).lv2

	install -m 644 *.so  $(DESTDIR)$(PREFIX)/lib/lv2/$(NAME).lv2/
	install -m 644 *.ttl $(DESTDIR)$(PREFIX)/lib/lv2/$(NAME).lv2/
	cp -rv modgui $(DESTDIR)$(PREFIX)/lib/lv2/$(NAME).lv2/
