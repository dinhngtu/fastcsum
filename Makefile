CPPFLAGS+=-MMD -MP -Iinclude
CXXFLAGS+=-Wall -Wextra -Wformat=2 -Werror=shadow -Werror=return-type -std=c++14 -fwrapv

USE_AVX?=0

ifeq ($(DEBUG), 1)
	CPPFLAGS+=-DDEBUG=1
	CFLAGS+=-O0 -g3 -fno-omit-frame-pointer
	CXXFLAGS+=-O0 -g3 -fno-omit-frame-pointer
else
	CPPFLAGS+=-DNDEBUG
	CFLAGS+=-O2 -g3
	CXXFLAGS+=-O2 -g3
endif

OBJECTS=\
	checksum-generic.o \
	checksum-x64-128b.o \
	checksum-x64-64b.o \
	checksum-adx.o \
	checksum-adx-v2.o \
	checksum-adx-align.o \
	checksum-adx-align2.o \
	checksum-avx2.o \

ifeq ($(USE_AVX), 1)
checksum-avx2.o: CXXFLAGS+=-mavx2
endif

DEPS=$(OBJECTS:.o=.d)

all: libfastcsum.a checksum-test

libfastcsum.a: $(OBJECTS)
	$(AR) rcs $@ $?

checksum-test: libfastcsum.a catch_amalgamated.o

check: checksum-test
	./$< --skip-benchmarks

clean:
	$(RM) checksum-test *.o *.d *.a

.PHONY: check clean

-include $(DEPS)
