all: build/ci

build/ci: src/main.cpp src/ci.cpp | build
	g++ -std=c++17 src/main.cpp src/ci.cpp -o build/ci -pthread

test: build/tests
	./build/tests

build/tests: tests/tests.cpp src/ci.cpp | build
	g++ -std=c++17 -I include tests/tests.cpp src/ci.cpp -o build/tests -DTESTING
	
build:
	mkdir -p build

clean:
	rm -rf build
