OPTS = -std=c++0x -Wall

EXPAND: Main.o LZExpander.o LZFactory.o
	g++ $(OPTS) -o EXPAND.exe Main.o LZExpander.o LZFactory.o

Main.o: Main.cpp LZExpander.h LZFactory.h
	g++ $(OPTS) -c Main.cpp

LZExpander.o: LZExpander.cpp LZExpander.h LZTypedefs.h CircularVector.h
	g++ $(OPTS) -c LZExpander.cpp

LZFactory.o: LZFactory.cpp LZFactory.h LZExpander.h
	g++ $(OPTS) -c LZFactory.cpp

clean:
	rm *.o EXPAND.exe
