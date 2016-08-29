
SOURCES:=$(shell find src -name "*.cpp" | grep -v "^src/bin.*")
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)
HEADERS:=$(shell find src -name "*.h")

all: libpubsub.so publish subscribe throughput throughput_shm throughput_pipe examples

examples: example1 example2 example3 example4 example5 example6

example%: test/example%.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

libpubsub.so: $(OBJECTS)
	g++ -g -pthread -shared -o $@ $^ -std=c++11 -lrt

publish: bin/bin/publish.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
subscribe: bin/bin/subscribe.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
throughput: bin/bin/throughput.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
throughput_shm: bin/bin/throughput_shm.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
throughput_pipe: bin/bin/throughput_pipe.o
	g++ -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
test/%.o: test/%.cpp libpubsub.so
	g++ -g -fPIC -c -o $@ $< -Isrc -std=c++11

bin/%.o: src/%.cpp $(HEADERS)
	mkdir -p `dirname $@`
	g++ -g -fPIC -c -o $@ $< -std=c++11 -I./src
	
clean:
	rm -rf bin test/*.o ./example* libpubsub.so
	
