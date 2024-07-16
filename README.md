# BTL 2 - Game Devlopment

MingW for VSCode: https://code.visualstudio.com/docs/cpp/config-mingw

SDL2: https://github.com/libsdl-org/SDL/releases/tag/release-2.30.0 -> SDL2-devel-2.30.0-mingw.zip

To execute the game
'''
g++ -I SDL2-Lib/include -L SDL2-Lib/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
'''
Run main.exe after execution
