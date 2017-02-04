# GNU make for project 'seqpipe'

TARGET   := seqpipe
MODULES  := $(patsubst %.cpp,%,$(wildcard src/*.cpp))

#----------------------------------------------------------#
# compiler and flags

GITVER := $(shell git rev-parse --short HEAD)

CXX      ?= g++
CXXFLAGS ?= -std=c++11 -DGITVER=\"${GITVER}\" -Wall
LDFLAGS  ?= -pthread

ifeq ("${DEBUG}","1")
	CXXFLAGS += -g
else
	CXXFLAGS += -O2 -DNDEBUG
endif

#----------------------------------------------------------#
# build rules
.PHONY: all clean test

all: ${TARGET}

clean:
	@rm -fv ${TARGET} ${MODULES:%=tmp/%.o} ${MODULES:%=tmp/%.d}

test:
	@./tests/system/run.sh

${TARGET}: ${MODULES:%=tmp/%.o}
	${CXX} ${LDFLAGS} -o $@ $^

tmp/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

#----------------------------------------------------------#
# dependency auto checking

ifneq ("${MAKECMDGOALS}", "clean")
sinclude ${MODULES:%=tmp/%.d}
tmp/%.d: %.cpp
	@mkdir -p ${@D}
	@${CXX} -std=c++11 -MM $< -MT ${@:%.d=%.o} | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@
endif
