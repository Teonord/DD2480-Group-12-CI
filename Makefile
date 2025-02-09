# Define OS detection
OS := $(shell uname)

# Build the main executable
all: build/ci

build/ci: src/main.cpp src/ci.cpp | build
	g++ -std=c++17 -pthread -Wall -Wextra -Wpedantic src/main.cpp src/ci.cpp -o build/ci -lws2_32

# Run tests
test: build/tests
	./build/tests

build/tests: tests/tests.cpp src/ci.cpp | build
	g++ -std=c++17 -pthread -I include -Wall -Wextra -Wpedantic tests/tests.cpp src/ci.cpp -o build/tests -DTESTING -lws2_32

# Cross-platform build directory creation
build:
ifeq ($(OS), Windows_NT)
	@if not exist build mkdir build
else
	mkdir -p build
endif

# Cross-platform clean up
clean:
ifeq ($(OS), Windows_NT)
	@if exist build rmdir /s /q build
else
	rm -rf build
endif
