OPTS = -std=c++0x -Wall

LZ: Main.o LZCompressor.o LZFactory.o
	g++ $(OPTS) -o LZ.exe Main.o LZCompressor.o LZFactory.o

Main.o: Main.cpp LZCompressor.h LZFactory.h
	g++ $(OPTS) -c Main.cpp

LZCompressor.o: LZCompressor.cpp LZCompressor.h LZTypedefs.h CircularVector.h
	g++ $(OPTS) -c LZCompressor.cpp

LZFactory.o: LZFactory.cpp LZFactory.h LZCompressor.h
	g++ $(OPTS) -c LZFactory.cpp

clean:
	rm *.o LZ.exe
