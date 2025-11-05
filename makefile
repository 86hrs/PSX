.PHONY: b r all clean

b:
	cd build && cmake --build . --parallel 8

build_all:
	cd build && cmake .. && cmake --build . --parallel 8

r:
	cd build && cd bin && ./main

all: build_all r

clean:
	rm -rf build && mkdir build
