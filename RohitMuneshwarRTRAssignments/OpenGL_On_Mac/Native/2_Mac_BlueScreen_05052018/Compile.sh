mkdir -p plain_blue_window.app/Contents/MacOS

clang++ -o plain_blue_window.app/Contents/MacOS/plain_blue_window plain_blue_window.mm -framework Cocoa -framework QuartzCore -framework OpenGL
