
CXX_FLAGS = -Wall
SOURCES = $(wildcard *.cpp)
OBJS = $(addprefix build/,$(SOURCES:.cpp=.o))

all: build/AI.dll

build:
	mkdir build

build/%.o: %.cpp build
	g++ -c $< -o $@ $(CXX_FLAGS)

build/AI.dll: build $(OBJS)
	g++ $(OBJS) -o $@ $(CXX_FLAGS) -fPIC --shared

clean:
	rmdir /s /q build