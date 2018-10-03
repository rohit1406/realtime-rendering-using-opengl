compile :
cl.exe /c /EHsc abc.cpp

link:
link.exe abc.obj user32.lib gdi32.lib


GLUT-
compile:
cl.exe /c /EHsc /I C:\freeglut\include ConcentricTrianglesStructure.cpp

link:
link.exe ogl.obj /LIBPATH:C:\freeglut\lib freeglut.lib /SUBSYSTEM:CONSOLE

GLEW-
compile:
cl.exe /c /EHsc /I C:\glew\include window.cpp

link:
link.exe program.obj /LIBPATH:C:\glew\lib\Release\Win32 glew32.lib

link.exe window.obj /LIBPATH:C:\glew\lib\Release\Win32 glew32.lib d3d11.lib D3dcompiler.lib DirectXTK.lib /NODEFAULTLIB:msvcrt.lib


Compile and Link at one go:
cl.exe /EHsc abc.cpp /link user32.lib gdi32.lib ab.res

cl.exe /EHsc window.cpp /LIBPATH:C:\glew\lib\Release\Win32 /link glew32.lib

ReadMe
1 compiling resource file
rc sugat.rc

2 compiling in a single line

cl.exe /EHsc .cpp file user32.lib gid32.lib sugat.res 

cl.exe /c /EHsc /I C:\glew\include *.cpp
link.exe *.obj *.res /LIBPATH:C:\glew\lib\Release\Win32 glew32.lib


====================================================================


Assignments:

1. WndProc madhe 4 messages add kara WM_CREATE, WM_LBUTTONDOWN, WM_KEYDOWN.
LBUTTONDOWN madhe kontya co-ordinate war click zalay te dakhawa ("WMKEYDOWN Received co-ordinate");

2. FullScreen

3. FullScreen with changing color on key press
R,G,B, C, M, Y, K, W
black 0 0 0
white 255 255 255
red 255 0 0
green 0 255 0
blue 0 0 255
cyan 0 255 255
mangenta 255 0 255
yellow 255 255 0

4. Fullscreen dusrya padhdhtine krta yeil ka te paha

5. Window 800*600 chi kara and center la ana

5. Window 800*600 chi kara and center la ana with negation of taskbar

6. Change program to double buffer

Draw Graph paper

7. Draw Triangle

8. Draw Square

9. Draw Circle

10. Ataparyantchya sarv assignments GLUT madhe kara

draw single point in the center of the window and user GL_POINT_SIZE to make it larger (just like GL_line width)

Draw multicolored triangle without grid

Draw a rectangle of color corn flower (ha directx cha flagship color ahe)

draw a shape of kundli

draw 10 concentric triangles

draw 10 concentric rectangles

draw 10 concentric circles

draw deathly hollow sign


1 triangle baswa 100 chya viewing volume madhe
-50 50 -50 50 -50 50
glOrtho

triangle cha code jashas tasa copy kara and glOrtho chya jagi glFrustum liha
-1 1 -1 1 -1 1

to keep square as a square and not as rectangle w.r.t. resizing the window, multiply bottom and top while resizing.

inspite of calling warmup call in initialize to avoid  cutting of appex and right size of triangle, do modelling transformation by pushing/translating the object deep into the screen by -3

use glTranslate

use gluLookAt

triangle and square draw kara using gluPerspective

animated triangle and square draw kara

viewport assignments

pyramid and cube

animated pyramid and cube

single revolving cube without using any transformation functions like loadIdentity, scalef,translatef, rotatef, matrixmode

Solar system

Robotic Hand


07012018 - find which extensions my graphics project supports - first PP program
21012018 -Second PP program