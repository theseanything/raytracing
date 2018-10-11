clean:
	-rm ./bin/main

./bin/main: ./src/main.cpp
	-mkdir -p bin
	clang++ -std=c++14 -o ./bin/main ./src/main.cpp

./bin/picture.ppm: ./bin/main
	./bin/main

picture: ./bin/picture.ppm
	open ./bin/picture.ppm

run: ./bin/main
	./bin/main

install:
	sudo apt-get install build-essential

ubuntu: 
	-mkdir -p bin
	g++ -std=c++14 -pthread -o ./bin/main ./src/main.cpp
	./bin/main