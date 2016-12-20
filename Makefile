
SOURCES:=$(shell find src -name "*.cpp" | grep -v "^src/bin.*")
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)
HEADERS:=$(shell find src -name "*.h")

CXX:=g++

all: libpubsub.so publish subscribe throughput pubsub_dump examples mesh

examples:

example%: test/example%.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..
	
mesh: test/mesh.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..


libpubsub.so: $(OBJECTS)
	$(CXX) -g -pthread -shared -o $@ $^ -std=c++11 -lrt -lwebsockets

publish: bin/bin/publish.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

subscribe: bin/bin/subscribe.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

pubsub_dump: bin/bin/pubsub_dump.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

throughput: bin/bin/throughput.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

throughput_shm: bin/bin/throughput_shm.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

throughput_pipe: bin/bin/throughput_pipe.o
	$(CXX) -g -pthread -o $@ $< -L.. -L. -lpubsub -Wl,-rpath=. -Wl,-rpath=..

test/%.o: test/%.cpp libpubsub.so
	$(CXX) -g -fPIC -c -o $@ $< -Isrc -std=c++11

bin/%.o: src/%.cpp $(HEADERS)
	mkdir -p `dirname $@`
	$(CXX) -g -fPIC -c -o $@ $< -std=c++11 -I./src

clean:
	rm -rf bin test/*.o ./example* libpubsub.so

install:
	cp -f publish /usr/bin
	cp -f subscribe /usr/bin
	cp -f pubsub_dump /usr/bin
	cp -f libpubsub.so /usr/lib
	cp -f src/libpubsub.h /usr/include


deploy:
	rsync -v -r . 192.168.1.1:libpubsub