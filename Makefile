
CXX_FLAGS = -Wall -g -O2
SOURCES = $(wildcard *.cpp)
OBJS = $(addprefix build/,$(SOURCES:.cpp=.o))

all: build build/AI.dll  build/test.exe

build:
	mkdir build

build/%.o: %.cpp build
	g++ -c $< -o $@ $(CXX_FLAGS)

build/AI.dll: build/AIEngine.o build/Strategy.o
	g++ $^ -o $@ $(CXX_FLAGS) -fPIC --shared

build/test.exe: build/AIEngine.o build/test.o
	g++ $^ -o $@ $(CXX_FLAGS)

clean:
	rmdir /s /q build