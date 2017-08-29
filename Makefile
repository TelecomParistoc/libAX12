TARGET = libAX12.so
SRCS = ax12driver.c ax-comm.c
HEADERS = $(addprefix src/, ${SRCS:.c=.h})
OBJECTS = $(addprefix build/,${SRCS:.c=.o})
TESTS = tests/AX12position tests/AXcomm tests/AXmove

JSBINDINGS := $(wildcard JSbinding/*.js)

PYTHON_BINDING = PythonBinding/ax12driver.py
PYTHON_PREFIX = /usr/local/lib/python2.7/dist-packages/
LOCAL_PYTHON = AX12.py

CC = gcc
CFLAGS = -O2 -std=gnu99 -Wall -Werror -fpic
LDFLAGS= -shared -lwiringPi -lm -lrobotutils
PREFIX = /usr/local/
VPATH = build/

vpath %.c src/ tests/
vpath %.h src/

.PHONY: all build clean tests AX12console jsinstall

all: build build/$(TARGET)

build:
	@mkdir -p build
build/%.o: %.c build/%.d
	@echo "$<"
	@$(CC) -c -o $@ $< $(CFLAGS)
build/%.d : %.c
	@$(CC) $(CFLAGS) -MM -MF $@ -MP $<

build/$(TARGET): $(OBJECTS)
	@echo "\nLinking target $@"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

tests: LDFLAGS=-lAX12
tests: $(TESTS)

clean:
	rm -f build/*.o build/*.so build/*.d
	rm -f $(TESTS)

jsinstall: $(JSBINDINGS) JSbinding/package.json
	mkdir -p $(DESTDIR)$(PREFIX)/lib/node_modules/AX12
	cp -r JSbinding/* $(DESTDIR)$(PREFIX)/lib/node_modules/AX12
	cd $(DESTDIR)$(PREFIX)/lib/node_modules/AX12; npm install
AX12console: AX12console/app.js AX12console/package.json AX12console/AX12
	mkdir -p $(DESTDIR)$(PREFIX)/lib/node_modules/AX12console
	cp -r AX12console/* $(DESTDIR)$(PREFIX)/lib/node_modules/AX12console
	cd $(DESTDIR)$(PREFIX)/lib/node_modules/AX12console; npm install
	cp AX12console/AX12 $(DESTDIR)$(PREFIX)/bin/
	chmod a+x $(DESTDIR)$(PREFIX)/bin/AX12

pythoninstall:
	cp $(PYTHON_BINDING) $(PYTHON_PREFIX)$(LOCAL_PYTHON)
	sudo python -c 'content = open("$(PYTHON_PREFIX)$(LOCAL_PYTHON)", "rb").read().replace("LIBNAME", "\"$(DESTDIR)$(PREFIX)/lib/$(TARGET)\""); open("$(PYTHON_PREFIX)$(LOCAL_PYTHON)", "w+").write(content)';

install: build/$(TARGET) jsinstall AX12console pythoninstall
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/AX12
	cp build/$(TARGET) $(DESTDIR)$(PREFIX)/lib/
	cp $(HEADERS) $(DESTDIR)$(PREFIX)/include/AX12/
	ln -s -f $(DESTDIR)$(PREFIX)/include/AX12/ax12driver.h  $(DESTDIR)$(PREFIX)/include/AX12/ax12.h
	chmod 0755 $(DESTDIR)$(PREFIX)/lib/$(TARGET)
	ldconfig
	ldconfig -p | grep AX12

-include $(subst .c,.d,$(SRCS))
