.PHONY: all 

T_CC_FLAGS       ?= $(shell sdl-config --cflags) -std=c++0x -Wall -Isrc -I../see/src 
T_CC_OPT_FLAGS   ?= -O0
T_CC_DEBUG_FLAGS ?= -g

SRCFILES:= $(shell find src '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.cpp" ')' | sed -e 's!^\./!!g') ../see/src/cpp_binding/script.cpp \

include ${T_BASE}/utl/template.mk

all: ${T_OBJ}/${PRJ}.a

-include ${DEPFILES}

${T_OBJ}/${PRJ}.a: ${OBJFILES} ${T_OBJ}/see.a
	@echo AR $@
	${V}ar r $@ $^
