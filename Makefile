clean:
	-rm ./bin/main

main:
	clang++ -o ./bin/main ./src/main.cpp

picture: main
	./bin/main > ./bin/picture.ppm
	open ./bin/picture.ppm