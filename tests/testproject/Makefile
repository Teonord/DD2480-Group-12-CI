all: build/prog

build/prog: prog.cpp | build
	g++ -std=c++17 prog.cpp -o build/prog

build:
	mkdir -p build

clean:
	rm -rf build