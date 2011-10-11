.PHONY: all stat-loc clean

V       ?= @
E_ENCODE = $(shell echo $(1) | sed -e 's!_!_1!g' -e 's!/!_2!g')
E_DECODE = $(shell echo $(1) | sed -e 's!_1!_!g' -e 's!_2!/!g')
T_BASE   = target

T_FLAGS_OPT  ?= -O0 -g
T_CXX_FLAGS  ?= $(shell sdl-config --cflags) ${T_FLAGS_OPT} -I/home/xinhaoyuan/include -std=c++0x -D__LINUX__
T_CC_FLAGS   ?= $(shell sdl-config --cflags) ${T_FLAGS_OPT} -I/home/xinhaoyuan/include -std=c99   -D__LINUX__
T_LINK_FLAGS ?= $(shell sdl-config --libs) -lpthread -g /home/xinhaoyuan/see/target/see.a

SRCFILES:= $(shell find src '(' '!' -regex '^\./_.*' ')' -and '(' -iname "*.cpp" -or -iname "*.c" ')' | sed -e 's!\./!!g')
OBJFILES:= $(addprefix ${T_BASE}/,$(addsuffix .o,$(foreach FILE,${SRCFILES},$(call E_ENCODE,${FILE}))))
DEPFILES:= $(OBJFILES:.o=.d)

all: ${T_BASE}/test

stat-loc:
	${V}wc ${SRCFILES} -l

clean:
	-${V}rm -f target/*

-include ${DEPFILES}

${T_BASE}/%.cpp.d:
	@echo DEP $(call E_DECODE,$*).cpp
	${V}${CXX} $(call E_DECODE,$*).cpp -o$@ -MM $(T_CXX_FLAGS) -MT $(@:.d=.o)

${T_BASE}/%.cpp.o: ${T_BASE}/%.cpp.d
	@echo CXX $(call E_DECODE,$*).cpp
	${V}${CXX} $(call E_DECODE,$*).cpp -o$@ ${T_CXX_FLAGS} -c

${T_BASE}/%.c.d:
	@echo DEP $(call E_DECODE,$*).c
	${V}${CXX} $(call E_DECODE,$*).c -o$@ -MM $(T_CXX_FLAGS) -MT $(@:.d=.o)

${T_BASE}/%.c.o: ${T_BASE}/%.c.d
	@echo CC $(call E_DECODE,$*).c
	${V}${CC} $(call E_DECODE,$*).c -o$@ ${T_CC_FLAGS} -c

${T_BASE}/test: ${OBJFILES}
	@echo LINK $@
	${V}${CXX} $^ -o$@ ${T_LINK_FLAGS}
