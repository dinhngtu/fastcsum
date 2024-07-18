CPPFLAGS+=-MMD -MP -Iinclude
CXXFLAGS+=-Wall -Wextra -Wformat=2 -Werror=shadow -Werror=return-type -std=c++14 -fwrapv -fno-strict-aliasing

ENABLE_AVX2?=0
ENABLE_AVX?=0
ENABLE_SSE41?=0
ARCH=
TUNE=

ifeq ($(DEBUG), 1)
	CPPFLAGS+=-DDEBUG=1
	CFLAGS+=-O0 -g3 -fno-omit-frame-pointer
	CXXFLAGS+=-O0 -g3 -fno-omit-frame-pointer
else
	CPPFLAGS+=-DNDEBUG
	CFLAGS+=-O2 -g3
	CXXFLAGS+=-O2 -g3
endif

ifneq ($(strip $(ARCH)),)
	CFLAGS+=-march=$(ARCH)
	CXXFLAGS+=-march=$(ARCH)
endif
ifneq ($(strip $(TUNE)),)
	CFLAGS+=-mtune=$(TUNE)
	CXXFLAGS+=-mtune=$(TUNE)
endif

TARGETS=\
	libfastcsum.a \
	test-fastcsum \
	fastcsum-version \

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
	checksum-vec256.o \
	checksum-vec128.o \

OBJECTS_AVX2=\
	checksum-avx2.o \
	checksum-vec256.o \
	checksum-vec128.o \

OBJECTS_AVX=\
	checksum-vec256.o \
	checksum-vec128.o \

OBJECTS_SSE41=\
	checksum-vec128.o \

ifeq ($(ENABLE_AVX2), 1)
CPPFLAGS+=-DFASTCSUM_ENABLE_AVX2=1
$(OBJECTS_AVX2): CXXFLAGS+=-mavx2
OBJECTS+=\
	checksum-avx2-v3.o \
	checksum-avx2-v5.o \
	checksum-avx2-v6.o \
	checksum-avx2-v7.o \

else ifeq ($(ENABLE_AVX), 1)
CPPFLAGS+=-DFASTCSUM_ENABLE_AVX=1
$(OBJECTS_AVX): CXXFLAGS+=-mavx

else ifeq ($(ENABLE_SSE41), 1)
CPPFLAGS+=-DFASTCSUM_ENABLE_SSE41=1
$(OBJECTS_SSE41): CXXFLAGS+=-msse4.1

endif

DEPS=$(OBJECTS:.o=.d)

all: $(TARGETS)

libfastcsum.a: $(OBJECTS)
	$(AR) rcs $@ $^

test-fastcsum: libfastcsum.a catch_amalgamated.o

fastcsum-version: libfastcsum.a

check: test-fastcsum
	./$< --skip-benchmarks

clean:
	$(RM) $(TARGETS) *.o *.d

.PHONY: check clean

-include $(DEPS)
