
CC = gcc
CXX = g++
CFLAGS = -Wall -O2
CXXFLAGS = -Wall -O2 -std=c++11 -DDEBUG
INCLUDES =
LFLAGS = -s
LIBS = -lm -lz

BUILD_DIR = ../build
SOURCES = $(wildcard *.cpp)
OBJS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

platepro: ${OBJS}
	${CXX} ${LFLAGS} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f $(BUILD_DIR)/*.o platepro

# c
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

# c++
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	${CXX} ${CXXFLAGS} ${INCLUDES} -c $< -o $@

# requires full groff package installed
manual:
	groff -m man -T pdf ../platepro.1 > ../manual.pdf

install: platepro
	install platepro /data/data/com.termux/files/usr/bin/

uninstall:
	rm /data/data/com.termux/files/usr/bin/platepro

installman:
	cp ../platepro.1 /data/data/com.termux/files/usr/share/man/man1

removeman:
	rm /data/data/com.termux/files/usr/share/man/man1/platepro.1
