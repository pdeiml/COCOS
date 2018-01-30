CC = g++

ROOTCXXFLAGS := $(shell root-config --cflags)
ROOTLIBFLAGS := $(shell root-config --libs)

CXXFLAGS = $(ROOTCXXFLAGS)
LIBFLAGS = $(ROOTLIBFLAGS) 

EXECUTES = COCOS COCOShpc

all: $(EXECUTES)

clean:
	rm $(EXECUTES)

# executables
COCOS: ./COCOS.cpp
	$(CC) ./COCOS.cpp $(CXXFLAGS) -o COCOS $(LIBFLAGS)

COCOShpc: ./COCOShpc.cpp
	$(CC) ./COCOShpc.cpp $(CXXFLAGS) -o COCOShpc $(LIBFLAGS)
