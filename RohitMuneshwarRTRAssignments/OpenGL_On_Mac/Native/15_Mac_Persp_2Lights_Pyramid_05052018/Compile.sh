mkdir -p window.app/Contents/MacOS

clang++ -o window.app/Contents/MacOS/window window.mm -framework Cocoa -framework QuartzCore -framework OpenGL
