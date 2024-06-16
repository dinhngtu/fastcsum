CXXFLAGS+=-O2 -std=c++14

checksum-test: checksum.o checksum-x64.o catch_amalgamated.o

clean:
	$(RM) checksum.o checksum-x64.o catch_amalgamated.o checksum-test

.PHONY: clean
