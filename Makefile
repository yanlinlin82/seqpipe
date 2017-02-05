# GNU make for project 'seqpipe'

TARGET  := seqpipe
MODULES := $(patsubst %.cpp,%,$(wildcard src/*.cpp))

UNIT_TEST    := tmp/unit-test
UNIT_MODULES := \
	src/CommandLineParser \
	src/StringUtils \
	src/System \
	$(patsubst %.cpp,%,$(wildcard tests/unit/*.cpp))

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
.PHONY: all clean unit test

all: ${TARGET}

clean:
	@rm -rfv ${TARGET} tmp/

unit: ${UNIT_TEST}
	@${UNIT_TEST}

test: unit ${TARGET}
	@./tests/system/run.sh

${TARGET}: ${MODULES:%=tmp/%.o}
	${CXX} ${LDFLAGS} -o $@ $^

${UNIT_TEST}: ${UNIT_MODULES:%=tmp/%.o}
	${CXX} ${LDFLAGS} -o $@ $^

tmp/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

#----------------------------------------------------------#
# dependency auto checking

ifneq ("${MAKECMDGOALS}", "clean")
sinclude ${MODULES:%=tmp/%.d}
sinclude ${UNIT_MODULES:%=tmp/%.d}
tmp/%.d: %.cpp
	@mkdir -p ${@D}
	@${CXX} -std=c++11 -MM $< -MT ${@:%.d=%.o} | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@
endif
