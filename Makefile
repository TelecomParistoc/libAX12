TARGET = libAX12.so
SRCS = ax12driver.c ax-comm.c
HEADERS = $(addprefix src/, ${SRCS:.c=.h})
OBJECTS = $(addprefix build/,${SRCS:.c=.o})
TESTS = tests/AX12position tests/AXcomm tests/AXmove

JSBINDINGS := $(wildcard JSbinding/*.js)

PYTHON_BINDING = PythonBinding/AX12*.py
PYTHON_UTIL = PythonBinding/I2C_bus.py
PYTHON_PREFIX = /usr/local/lib/python3.4/dist-packages/
LOCAL_PYTHON_BINDING = AX12.py
LOCAL_PYTHON_UTIL = I2C_bus.py

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
	@$(CC) -c -o $@ $< $(CFLAGS) $(EXTRAFLAGS)
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

pythoninstall: build/$(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp build/$(TARGET) $(DESTDIR)$(PREFIX)/lib/
	cp $(PYTHON_BINDING) $(PYTHON_PREFIX)
	sudo python -c 'content = open("$(PYTHON_PREFIX)$(LOCAL_PYTHON_BINDING)", "rb").read().replace("LIBNAME", "\"$(DESTDIR)$(PREFIX)/lib/$(TARGET)\""); open("$(PYTHON_PREFIX)$(LOCAL_PYTHON_BINDING)", "w+").write(content)';
	cp $(PYTHON_UTIL) $(PYTHON_PREFIX)$(LOCAL_PYTHON_UTIL)
	sudo python -c 'content = open("$(PYTHON_PREFIX)$(LOCAL_PYTHON_UTIL)", "rb").read().replace("LIBNAME", "\"$(DESTDIR)$(PREFIX)/lib/$(TARGET)\""); open("$(PYTHON_PREFIX)$(LOCAL_PYTHON_UTIL)", "w+").write(content)';

pythoninstall_simu:
	mkdir -p $(PYTHON_PREFIX)
	cp $(PYTHON_BINDING) $(PYTHON_PREFIX)
	cp $(PYTHON_UTIL) $(PYTHON_PREFIX)$(LOCAL_PYTHON_UTIL)

install: build/$(TARGET) jsinstall AX12console pythoninstall
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/AX12
	cp build/$(TARGET) $(DESTDIR)$(PREFIX)/lib/
	cp $(HEADERS) $(DESTDIR)$(PREFIX)/include/AX12/
	ln -s -f $(DESTDIR)$(PREFIX)/include/AX12/ax12driver.h  $(DESTDIR)$(PREFIX)/include/AX12/ax12.h
	chmod 0755 $(DESTDIR)$(PREFIX)/lib/$(TARGET)
	ldconfig
	ldconfig -p | grep AX12

ChibiOS:
	make EXTRAFLAGS=-DCHIBIOS

-include $(subst .c,.d,$(SRCS))
