all: build/prog

build/prog: prog.cpp | build
	g++ -std=c++17 prog.cpp -o build/prog

test: build/tests
	./build/tests

build/tests: tests.cpp | build
	g++ -std=c++17 tests.cpp -o build/tests

build:
	mkdir -p build

clean:
	rm -rf build
