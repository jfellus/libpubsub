
SOURCES:=$(shell find src -name "*.cpp")
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)
HEADERS:=$(shell find src -name "*.h")

all: libpubsub.so examples

examples: example1 example2 example3

example%: test/example%.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

libpubsub.so: $(OBJECTS)
	g++ -g -pthread -shared -o $@ $^ -std=c++11


test/%.o: test/%.cpp libpubsub.so
	g++ -g -fPIC -c -o $@ $< -Isrc -std=c++11

bin/%.o: src/%.cpp $(HEADERS)
	mkdir -p `dirname $@`
	g++ -g -fPIC -c -o $@ $< -std=c++11
	
clean:
	rm -rf bin test/*.o ./example* libpubsub.so
	
