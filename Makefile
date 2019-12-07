CC ?= gcc
CXX ?= g++

_FLAGS   ?= -Wall -Wextra -O3 -Wno-unused-parameter
CFLAGS   += $(_FLAGS) -fPIC -std=c99
CXXFLAGS += $(_FLAGS) -fPIC -std=c++11

NAME = setbfree-controller


.ONESHELL:

all: build
build: $(NAME).so

$(NAME).so: build/$(NAME).c.o
	$(CC) $^ $(LDFLAGS) -shared -Wl,--no-undefined -o $@

build/$(NAME).c.o: $(NAME).c
	mkdir -p build
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -rf build
	rm -f *.so

validate:
	find | grep \\.ttl | xargs sord_validate

package:
	mkdir -p build/$(NAME).lv2
	install -m 644 *.so  build/$(NAME).lv2/
	install -m 644 *.ttl build/$(NAME).lv2/
	cp -rv modgui build/$(NAME).lv2/

deploy: package
	cd build
	tar cz ${NAME}.lv2 | base64 | curl -F 'package=@-' http://192.168.51.1/sdk/install

install-user: package
	install -d ~/.lv2/$(NAME).lv2
	cp -rv build/${NAME}.lv2 ~/.lv2/

install: package
	install -d $(DESTDIR)$(PREFIX)/lib/lv2/$(NAME).lv2
	cp -rv build/${NAME}.lv2 $(DESTDIR)$(PREFIX)/lib/lv2/
