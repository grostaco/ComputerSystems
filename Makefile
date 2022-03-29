CC              := gcc
CXX             := g++
ASM             := nasm
LD              := ld

CSTD            ?= 99
CXXSTD          := 2a

OBJ             := $(addprefix obj/, $(addsuffix .o, $(basename $(shell find src \( -name "*.c" -o -name "*.cpp" -o -name "*asm" \) -type f -printf "%f\n"))))
DEPS            := $(patsubst %.o, %.d, $(OBJ))

override CFLAGS         := -Iinclude -std=gnu$(CSTD) 
override CFLAGS         += -Wall -Werror -Wextra -Wparentheses -Wunreachable-code
override CFLAGS         += -Wunused -Wmissing-field-initializers -Wswitch-default -Wuninitialized
override CFLAGS         += -Wpointer-arith -Wswitch-enum -Wredundant-decls -Wshadow -Wno-unused-result -fopenmp
override CXXFLAGS       := -Iinclude -std=gnu++$(CXXSTD)
override CXXFLAGS       += -Wall -Werror -Wextra -Wparentheses -Wmissing-declarations -Wunreachable-code
override CXXFLAGS       += -Wunused -Wmissing-field-initializers -Wswitch-default -Wuninitialized
override CXXFLAGS       += -Wpointer-arith -Wswitch-enum -Wredundant-decls -Wshadow
override ASMFLAGS       :=
override RELEASE_CFLAGS ?= -O3 -flto
override DEBUG_CFLAGS   ?= -ggdb3 -fsanitize=address,undefined,leak -fstack-protector 

override LDFLAGS        := -lpthread -lm

ifeq ($(DEBUG), 1)	
	override CFLAGS   += $(DEBUG_CFLAGS)
	override CXXFLAGS += $(DEBUG_CFLAGS)
else
	override CFLAGS   += $(RELEASE_CFLAGS)
	override CXXFLAGS += $(RELEASE_CFLAGS)
endif	

ifeq ($(STATIC_ANALYZE), 1)
	override CFLAGS += -fanalyzer
endif

.PHONY: all run clean build debug deploy dump dump_func strace gprof

all: bin/out 

debug: CFLAGS+=$(DEBUG_CFLAGS)
debug: CXXFLAGS+=$(DEBUG_CFLAGS)
debug: bin/out
	gdb -q --args bin/out $(ARGS)

build: bin/out

dump: bin/out
	@objdump -Mintel -d $< $(DUMPOPT)

dump_func: bin/out
	@gdb $< -batch -ex "disassemble $(FUNC)" 

strace: bin/out
	@strace $< $(ARGS)

gprof: CFLAGS+=-pg
gprof: CXXFLAGS+=-pg
gprof: bin/out
	@gprof $<

run: bin/out
	./bin/out $(ARGS)

bin/out: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) -MMD -c -o $@ $< $(CFLAGS) $(LDFLAGS) 
obj/%.o: src/%.cpp
	$(CXX) -MMD -c -o $@ $< $(CXXFLAGS) $(LDFLAGS)
obj/%.o: src/%.asm
	$(ASM) -MMD -c -o $@ $< $(ASMFLAGS) $(LDFLAGS)

deploy:
	mkdir -p bin
	mkdir -p src
	mkdir -p obj
	mkdir -p include

clean:
	rm -rf $(shell find obj -type f -name "*.o" -o -name "*.d")
	rm -rf $(shell find bin -type f ! -name "*.*")

-include ($(DEPS))