# GNU make for project 'seqpipe'

TARGET   := seqpipe
MODULES  := $(patsubst %.cpp,%,$(wildcard src/*.cpp))

#----------------------------------------------------------#
# compiler and flags

GITVER := $(shell git rev-parse --short HEAD)

CXX      ?= g++
#CXXFLAGS ?= -std=c++11 -Wall -O2 -DGITVER=\"${GITVER}\"
CXXFLAGS ?= -std=c++11 -g -DGITVER=\"${GITVER}\"
LDFLAGS  ?= -pthread

#----------------------------------------------------------#
# build rules

all: ${TARGET}

clean:
	@rm -fv ${TARGET} ${MODULES:%=%.o} ${MODULES:%=%.d}

${TARGET}: ${MODULES:%=%.o}
	${CXX} ${LDFLAGS} -o $@ $^

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

#----------------------------------------------------------#
# dependency auto checking

ifneq ("${MAKECMDGOALS}", "clean")
sinclude ${MODULES:%=%.d}
%.d: %.cpp
	@${CXX} -std=c++11 -MM $< -MT ${@:%.d=%.o} | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@
endif
