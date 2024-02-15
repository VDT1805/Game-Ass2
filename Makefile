all:
	g++ -I SDL2-Lib/include -L SDL2-Lib/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf