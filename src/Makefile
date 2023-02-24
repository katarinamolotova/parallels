WWW = -std=c++17 -Wall -Werror -Wextra -D
GAUSS = GAUSSALGORITHM
WINOGRAD = WINOGRADALGORITHM
ANT = ANTALGORITHM

all: clean

gauss: clean
	g++ $(WWW) $(GAUSS) main.cc interface/interface.cc helpers/matrix.cc helpers/matrix_parser.cc algorithms/GaussAlgorithm.cc
	./a.out

ant: clean
	g++ $(WWW) $(ANT) main.cc interface/interface.cc helpers/matrix.cc helpers/matrix_parser.cc algorithms/AntAlgorithm.cc
	./a.out

winograd: clean
	g++ $(WWW) $(WINOGRAD) main.cc interface/interface.cc helpers/matrix.cc helpers/matrix_parser.cc algorithms/WinogradAlgorithm.cc
	./a.out

clean:
	rm -rf *.o
	rm -rf a.out

check:
	cp ../materials/linters/.clang-format ./
	clang-format -n ./*.cc ./*/*.cc ./*/*.h
	rm -rf ./.clang-format
