SOURCES = $(wildcard *.cpp)

CHECK = `which scan-build`
UNCRUSTIFY = uncrustify

OBJECTS = $(subst .cpp,.o,$(SOURCES))

.PHONY: clean check uncrust

all: v89

v89: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) -lpthread -lGL -lglut

CXXFLAGS = -g -std=c++11

clean:
	rm -f *.o *.orig

check:
	$(CHECK) -stats -load-plugin alpha.cplusplus.VirtualCall -load-plugin alpha.deadcode.UnreachableCode -maxloop 20 -k --use-analyzer Xcode -o check xcodebuild

uncrust:
	$(UNCRUSTIFY) -c VirtualH89/uncrust.cfg --no-backup VirtualH89/Src/*.cpp VirtualH89/Src/*.h

ajo:
	g++ -W -Wall -std=c++11 VirtualH89/Src/*.cpp -Wno-deprecated-declarations -framework OpenGL -framework GLUT -pthread -o v89
	g++ -W -Wall -std=c++11 converter/h8d2h17raw.cpp -o h8d2h17raw
	./h8d2h17raw Adventure/advent.h8d Adventure/advent.h17raw
	cd Adventure; V89_CONFIG=v89rc ../v89
