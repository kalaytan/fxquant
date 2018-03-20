CPP=g++
OUT=fxquant
SRC=$(wildcard *.cpp) $(wildcard ./strategies/*.cpp)
CFLAGS=-ggdb -O0 -std=c++14 
INCLUDES=-I. $(shell pkg-config --cflags libmongocxx libcurl) $(shell pkg-config --cflags jsoncpp)
LIBS=$(shell pkg-config --libs libmongocxx libcurl) $(shell pkg-config --libs liblog4cxx) $(shell pkg-config --libs jsoncpp) -lstdc++ -lpthread
all:
	@$(CPP) -o $(OUT) $(SRC) $(CFLAGS) $(INCLUDES) $(LIBS)

