all: build
	cd build && make

build:
	mkdir -p build
	cd build && cmake ..
	