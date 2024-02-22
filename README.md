Hướng dẫn cài MingW cho VSCode: https://code.visualstudio.com/docs/cpp/config-mingw

Cài SDL2: https://github.com/libsdl-org/SDL/releases/tag/release-2.30.0 -> SDL2-devel-2.30.0-mingw.zip

Mở cmd trong thư mục chứa project, chạy g++ -I SDL2-Lib/include -L SDL2-Lib/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

Mở file main.exe sau khi chạy xong
