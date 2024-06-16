CXXFLAGS+=-O2 -std=c++14

checksum-test: checksum.o checksum-x64-128b.o checksum-x64-64b.o checksum-adx.o checksum-adx-v2.o catch_amalgamated.o

clean:
	$(RM) *.o checksum-test

.PHONY: clean
