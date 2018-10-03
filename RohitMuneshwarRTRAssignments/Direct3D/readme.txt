cl.exe /c /EHsc /I C:\glew\include window.cpp

link.exe window.obj /LIBPATH:C:\glew\lib\Release\Win32 glew32.lib d3d11.lib D3dcompiler.lib 
user32.lib gdi32.lib

