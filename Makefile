CXXFLAGS+=-O2 -std=c++14

checksum-test: \
	checksum-generic.o \
	checksum-x64-128b.o \
	checksum-x64-64b.o \
	checksum-adx.o \
	checksum-adx-v2.o \
	checksum-adx-align.o \
	checksum-adx-align2.o \
	checksum-avx2.o \
	catch_amalgamated.o

checksum-avx2.o: CXXFLAGS+=-mavx2

clean:
	$(RM) *.o checksum-test

.PHONY: clean
