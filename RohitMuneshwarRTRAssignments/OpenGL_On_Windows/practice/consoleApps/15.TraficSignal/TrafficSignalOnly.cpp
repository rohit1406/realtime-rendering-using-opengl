//double buffer opengl program - 3D
#include<Windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<math.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
bool gbDone = false;
bool gbFullScreen = false;
HWND ghwnd;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)}; //for fullscreen

//changes for opengl
bool gbActiveWindow = false;
bool gbEscapePressed = false;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

GLUquadric *quadric = NULL;
//circle
GLfloat PI = 3.1415;
GLfloat CIRCLE_SIZE = 0.2f;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLIne, int iCmdShow){
	void initialize(void);
	void uninitialize(void);
	void display(void);//double buffer sathi
	
	WNDCLASSEX wndclass;
	TCHAR AppName[] = TEXT("Window Custom");
	TCHAR WinName[] = TEXT("Trafic Signal");
	HWND hwnd;
	MSG msg;
	RECT rect;
	//initialize window class
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //change for opengl
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = AppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	//initialization complete
	
	RegisterClassEx(&wndclass); //Register Class
	
	SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	int width = (rect.right-rect.left)/2 - WIN_WIDTH/2;
	int height = (rect.bottom - rect.top)/2 - WIN_HEIGHT/2;
	//int width = GetSystemMetrics(SM_CXSCREEN)/2 - WIN_WIDTH/2;
	//int height = GetSystemMetrics(SM_CYSCREEN)/2 - WIN_HEIGHT/2;
	
	
	//create window in memory
	hwnd = CreateWindowEx(  //change for opengl
	WS_EX_APPWINDOW,
	AppName,
	WinName,
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, //change for opengl
	width,
	height,
	WIN_WIDTH,
	WIN_HEIGHT,
	NULL,
	NULL,
	hInstance,
	NULL
	);
	ghwnd = hwnd;
	ShowWindow(hwnd, iCmdShow); //Show Window
	//ShowWindow(hwnd, SW_MAXIMIZE); //Show Window
	
	//update window removed, not necessary for opengl
	//UpdateWindow(hwnd); // color/paint the background of the window
	//added below two for opengl
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	
	initialize(); //opengl
	//message loop
	while(gbDone==false){
		
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				gbDone = true;
			}else{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}else{
			//rendering starts here
			//opengl change
			if(gbActiveWindow == true){
				if(gbEscapePressed == true){
					gbDone = true;
				}
				
				display(); //for double buffer
			}
		}
	}
	
	uninitialize(); //opengl
	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam){
	void ToggleFullscreen(void);
	void resize(int, int);
	//void display(void);
	void uninitialize(void);
	switch(iMsg){
		//case for opengl
		case WM_ACTIVATE:
			if(HIWORD(wParam)==0){
				gbActiveWindow = true;
			}else{
				gbActiveWindow = false;
			}
		break;
		//case added for opengl
		case WM_SIZE:
			resize(LOWORD(lParam),HIWORD(lParam));
		break;
		//WM_PAINT ch painting single buffer ahe. So u will face screen tearing/flickering (state save nhi krta yet - single state)
		/*case WM_PAINT:
			display(); //opengl
		break;*/ //removed as it is not needed for double buffering
		//for opengl
		case WM_CLOSE:
			uninitialize();
		break;
		//for opengl
		/*case WM_ERASEBKGND:
			return (0);
		break;*/ //removed as not needed for double buffering
		case WM_KEYDOWN:
			switch(wParam){
				
				case VK_ESCAPE:
					gbEscapePressed = true;
					break;
				break;
				case 0x46: //F or f key
						ToggleFullscreen();
				break;
			}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullscreen(){
	HMONITOR hMonitor;
	//check kara window fullscreen ahe ka
	if(gbFullScreen == false){
		//get current window style
		 dwStyle = GetWindowLong(ghwnd,GWL_STYLE);
		 
		 //does it includes overlapped window
		if(dwStyle & WS_OVERLAPPEDWINDOW){
			
			//evaluate current window placement
			bool bWindowPlacement = GetWindowPlacement(ghwnd,&wpPrev);
			
			//take the information of the window which you want to make full screen
			hMonitor = MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY);
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			bool bMonitorInfo = GetMonitorInfo(hMonitor, &mi);
			
			if(bWindowPlacement && bMonitorInfo){
				//set window style accordingly
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				//set window position accordingly
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right - mi.rcMonitor.left), 
				(mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			//hide the cursor
			ShowCursor(FALSE);
			gbFullScreen = true;
		}
	}else{
		//code to restore the full screen
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle & WS_OVERLAPPEDWINDOW);
		//set window placement back to previous
		SetWindowPlacement(ghwnd, &wpPrev);
		//set window position
		SetWindowPos(ghwnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		//show cursor
		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}

void initialize(){
	void resize(int,int);
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	//initialization of pfd
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; //for double buffer
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32; //added to make program 3d
	ghdc = GetDC(ghwnd);
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if(iPixelFormatIndex == 0){
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	
	if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd)==FALSE){
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	
	ghrc = wglCreateContext(ghdc);
	if(ghrc == NULL){
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	
	if(wglMakeCurrent(ghdc,ghrc)== FALSE){
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	//added for 3D support
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	//added for 3D support but are optional
	//glShadeModel(GL_SMOOTH);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//removed as not needed for double buffering
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void display(){
	void drawTrafficSignal(void);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //GL_DEPTH_BUFFER_BIT added for 3D support
	
	drawTrafficSignal();
	SwapBuffers(ghdc);
}

void drawTrafficSignal(){
	void drawRCircle(void);
	void drawGCircle(void);
	void drawYCircle(void);
	void drawQuad(void);
	void drawQuad1(void);
	void drawQuad2(void);
	void drawQuad3(void);
	void drawQuad4(void);
	void drawQuad5(void);
	void drawQuad6(void);
	void drawPipe(void);
	void drawHorizontalRoad(void);
	GLfloat zTrans = -5.0f;
	GLfloat yTrans = 0.70f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,yTrans+0.0f,zTrans);
	drawQuad();
	drawQuad1();
	drawQuad2();
	drawQuad3();
	drawQuad4();
	drawQuad5();
	drawQuad6();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,yTrans+0.5f,zTrans);
	drawRCircle();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,yTrans+0.0f,zTrans);
	drawYCircle();
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,-0.5f+yTrans,zTrans);
	drawGCircle();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,-0.85f+yTrans,zTrans);
	drawPipe();
	
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,yTrans+0.0f,zTrans);
	drawHorizontalRoad();*/
}
void drawPipe(){
	glBegin(GL_QUADS);
	glColor3f(0.75f,0.75,0.75f);
	glVertex3f(0.1f,0.1f,0.0f);
	glVertex3f(-0.1f,0.1f,0.0f);
	glVertex3f(-0.1f,-1.0f,0.0f);
	glVertex3f(0.1f,-1.0f,0.0f);
	glEnd();
}
void drawSphere(){
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	quadric = gluNewQuadric();
	glColor3f(1.0f,1.0f,0.0f);
	glScalef(0.0f,0.0f,0.0f);
	gluSphere(quadric, 0.75f,30,30);
}
void drawRCircle(){
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f,0.0f,0.0f);
	for(float angle=0.0f;angle<(2.0f*PI); angle = angle + 0.01){
		glVertex3f(cos(angle)*CIRCLE_SIZE,sin(angle)*CIRCLE_SIZE,0.0f);
	}
	glEnd();
}

void drawQuad(){
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(0.3f,0.75f,0.0f);
	glVertex3f(-0.3f,0.75f,0.0f);
	glVertex3f(-0.3f,-0.75f,0.0f);
	glVertex3f(0.3f,-0.75f,0.0f);
	glEnd();
}

void drawQuad1(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(0.3f,0.75f-sideMargin,0.0f);
	glVertex3f(0.3f,0.75f-lPg - sideMargin,0.0f);
	glVertex3f((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f,0.75f-lPg - sideMargin,0.0f);
	glVertex3f(0.30f+(0.75f-lPg - sideMargin)*3/4,0.75f - sideMargin,0.0f);
	
	
	glEnd();
}

void drawQuad2(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	GLfloat gap = 0.175f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(0.3f,0.75f-sideMargin-lPg-gap,0.0f);
	glVertex3f(0.3f,0.75f-lPg-lPg-sideMargin-gap,0.0f);
	glVertex3f((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f,0.75f-lPg-lPg-sideMargin-gap,0.0f);
	glVertex3f(0.30f+(0.75f-lPg - sideMargin)*3/4,0.75f-sideMargin-lPg-gap,0.0f);
	
	
	glEnd();
}

void drawQuad3(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	GLfloat gap = 0.175f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(0.3f,0.75f-sideMargin-lPg-gap-lPg-gap,0.0f);
	glVertex3f(0.3f,0.75f-lPg-lPg-lPg-sideMargin-gap-gap,0.0f);
	glVertex3f((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f,0.75f-lPg-lPg-lPg-sideMargin-gap-gap,0.0f);
	glVertex3f(0.30f+(0.75f-lPg - sideMargin)*3/4,0.75f-sideMargin-lPg-gap-lPg-gap,0.0f);
	
	
	glEnd();
}

void drawQuad4(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(-0.3f,0.75f-sideMargin,0.0f);
	glVertex3f(-0.3f,0.75f-lPg - sideMargin,0.0f);
	glVertex3f(-((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f),0.75f-lPg - sideMargin,0.0f);
	glVertex3f(-(0.30f+(0.75f-lPg - sideMargin)*3/4),0.75f - sideMargin,0.0f);
	
	
	glEnd();
}

void drawQuad5(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	GLfloat gap = 0.175f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(-0.3f,0.75f-sideMargin-lPg-gap,0.0f);
	glVertex3f(-0.3f,0.75f-lPg-lPg-sideMargin-gap,0.0f);
	glVertex3f(-((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f),0.75f-lPg-lPg-sideMargin-gap,0.0f);
	glVertex3f(-(0.30f+(0.75f-lPg - sideMargin)*3/4),0.75f-sideMargin-lPg-gap,0.0f);
	
	
	glEnd();
}

void drawQuad6(){
	GLfloat sideMargin = 0.05f;
	GLfloat lPg = 0.35f;
	GLfloat tPg = 0.45f;
	GLfloat gap = 0.175f;
	glBegin(GL_QUADS);
	glColor3f(0.70f,0.70f,0.70f);
	glVertex3f(-0.3f,0.75f-sideMargin-lPg-gap-lPg-gap,0.0f);
	glVertex3f(-0.3f,0.75f-lPg-lPg-lPg-sideMargin-gap-gap,0.0f);
	glVertex3f(-((GLfloat)(((0.75f-lPg - sideMargin)*3/4)/2)+0.30f),0.75f-lPg-lPg-lPg-sideMargin-gap-gap,0.0f);
	glVertex3f(-(0.30f+(0.75f-lPg - sideMargin)*3/4),0.75f-sideMargin-lPg-gap-lPg-gap,0.0f);
	
	
	glEnd();
}
void drawGCircle(){
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.0f,1.0f,0.0f);
	for(float angle=0.0f;angle<(2.0f*PI); angle = angle + 0.0001){
		glVertex3f(cos(angle)*CIRCLE_SIZE,sin(angle)*CIRCLE_SIZE,0.0f);
	}
	glEnd();
}

void drawYCircle(){
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f,1.0f,0.0f);
	for(float angle=0.0f;angle<(2.0f*PI); angle = angle + 0.0001){
		glVertex3f(cos(angle)*CIRCLE_SIZE,sin(angle)*CIRCLE_SIZE,0.0f);
	}
	glEnd();
}

void drawHorizontalRoad(){
	glBegin(GL_QUADS);
	glColor3f(0.5f,0.5f,0.5f);
	glVertex3f(-0.3f,0.0f,-1.0f);
	glVertex3f(0.3f,0.0f,-1.0f);
	glVertex3f(-0.3f,0.0f,1.0f);
	glVertex3f(0.3f,0.0f,1.0f);
	glEnd();
}

void resize(int width,int height){
	if(height == 0){
		height = 1;
	}
	
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat) width/ (GLfloat) height, 0.1f,100.0f);
}

void uninitialize(){
	if(gbFullScreen == true){
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd,GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
	
	wglMakeCurrent(NULL,NULL);
	wglDeleteContext(ghrc);
	ghrc = NULL;
	
	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;
	
	DestroyWindow(ghwnd);
	ghwnd = NULL;
}