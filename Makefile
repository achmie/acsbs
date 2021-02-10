CXX=g++
CXXFLAGS=-I. -Wall -O2
LIBS=-lm -lz

%.o: %.cpp
		$(CXX) -c -o $@ $< $(CXXFLAGS)

all: entropy speed statistics

entropy: entropy.o compstat.o
		$(CXX) -o $@ $^ $(LIBS)

speed: speed.o compstat.o compress.o
		$(CXX) -o $@ $^ $(LIBS)

statistics: statistics.o compstat.o
		$(CXX) -o $@ $^ $(LIBS)

clean:
		rm *.o

distclean: clean
		rm entropy speed statistics
