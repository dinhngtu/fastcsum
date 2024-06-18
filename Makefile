CPPFLAGS+=-MMD -MP -Iinclude
CXXFLAGS+=-Wall -Wextra -Wformat=2 -Werror=shadow -Werror=return-type -std=c++14 -fwrapv

USE_AVX2?=0

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
	checksum-generic64.o \
	checksum-x64-128b.o \
	checksum-x64-64b.o \
	cpuid.o \
	checksum-adx.o \
	checksum-adx-v2.o \
	checksum-adx-align.o \
	checksum-adx-align2.o \
	checksum-avx2.o \
	checksum-avx2-v3.o \
	checksum-avx2-v5.o \
	checksum-avx2-v6.o \
	checksum-vec256.o \

ifeq ($(USE_AVX2), 1)
cpuid.o checksum-avx2.o checksum-vec256.o: CXXFLAGS+=-mavx2
endif

DEPS=$(OBJECTS:.o=.d)

all: libfastcsum.a test-fastcsum

libfastcsum.a: $(OBJECTS)
	$(AR) rcs $@ $?

test-fastcsum: libfastcsum.a catch_amalgamated.o

check: test-fastcsum
	./$< --skip-benchmarks

clean:
	$(RM) test-fastcsum *.o *.d *.a

.PHONY: check clean

-include $(DEPS)
