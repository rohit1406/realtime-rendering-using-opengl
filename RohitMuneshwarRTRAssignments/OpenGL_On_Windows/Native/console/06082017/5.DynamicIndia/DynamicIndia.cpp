//double buffer opengl program - 3D
#include<Windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>

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

GLfloat gfFirstIAngle = -2.0f;
GLfloat gfFirstNAngle = 2.0f;
GLfloat gfSecondIAngle = -2.0f;
GLfloat gfAAngle = 2.0f;
GLfloat gfDColor = 0.0f;
GLfloat gfX1CentralLine = -2.5f;
GLfloat gfX2CentralLine = -2.5f;
GLfloat gfCentralTransform =  -2.0f;

bool gb1Pressed = false;
bool gb2Pressed = false;
bool gb3Pressed = false;
bool gb4Pressed = false;
bool gb5Pressed = false;
bool gb6Pressed = false;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLIne, int iCmdShow){
	void initialize(void);
	void uninitialize(void);
	void display(void);//double buffer sathi
	void updateFirstI(void);
	void updateN(void);
	void updateSecondI(void);
	void updateA(void);
	void updateDColor(void);
	void updateX2Line(void);
	void updateTransform(void);
	WNDCLASSEX wndclass;
	TCHAR AppName[] = TEXT("Window Custom");
	TCHAR WinName[] = TEXT("Dynamic India");
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
				if(gb1Pressed==true){
					updateFirstI();
					//gb1Pressed=false;
				} if(gb2Pressed==true){
					updateN();
					//gb2Pressed = false;
				} if(gb3Pressed==true){
					updateSecondI();
					//gb3Pressed=false;
				} if(gb4Pressed==true){
					updateA();
					//gb4Pressed=false;
				} if(gb5Pressed==true){
					updateDColor();
					//gb5Pressed=false;
				} if(gb6Pressed==true){
					updateTransform();
					updateX2Line();
					//gb6Pressed=false;
				}
				
				display(); //for double buffer
			}
		}
	}
	
	uninitialize(); //opengl
	return (int) msg.wParam;
}

void updateX1Line(){
	gfX1CentralLine = gfX1CentralLine + 0.001f;
	/*if(gfCentralTransform>=1.0f && gfX1CentralLine<=-0.1f){
		gfX1CentralLine = gfCentralTransform - 0.001f;
	}*/
	if(gfX1CentralLine>=-0.1f){
		gfX1CentralLine = -0.1f;
	}
}

void updateX2Line(){
	gfX2CentralLine = gfX2CentralLine + 0.001f;
	/*if(gfCentralTransform>=1.0f && gfX1CentralLine<=-0.1f){
		gfX1CentralLine = gfCentralTransform - 0.001f;
	}*/
	if(gfX2CentralLine>=0.1f){
		gfX2CentralLine = 0.1f;
		updateX1Line();
	}
}

void updateTransform(){
	gfCentralTransform = gfCentralTransform + 0.001f;
	if(gfCentralTransform>=1.0f){
		gfCentralTransform = 1.0f;
	}
}
void updateDColor(){
	gfDColor = gfDColor + 0.001f;
	if(gfDColor>= 0.5f){
		gfDColor= 0.5f;
	}
}
void updateFirstI(){
	gfFirstIAngle = gfFirstIAngle + 0.001f;
	if(gfFirstIAngle>= -1.0f){
		gfFirstIAngle= -1.0f;
	}
}

void updateN(){
	gfFirstNAngle = gfFirstNAngle - 0.001f;
	if(gfFirstNAngle<= 0.0f){
		gfFirstNAngle= 0.0f;
	}
}

void updateSecondI(){
	gfSecondIAngle = gfSecondIAngle + 0.001f;
	if(gfSecondIAngle>= 0.0f){
		gfSecondIAngle= 0.0f;
	}
}

void updateA(){
	gfAAngle = gfAAngle - 0.001f;
	if(gfAAngle <= 1.0f){
		gfAAngle= 1.0f;
	}
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
				case 0x30: //press 0 for reset
					gfFirstIAngle = -2.0f;
					gfFirstNAngle = 2.0f;
					gfSecondIAngle = -2.0f;
					gfAAngle = 2.0f;
					gfDColor = 0.0f;
					gfX1CentralLine = -2.5f;
					gfX2CentralLine = -2.5f;
					gfCentralTransform = -2.0f;
				break;
				case 0x31:
					gb1Pressed = true;
				break;
				case 0x32:
					gb2Pressed = true;
				break;
				case 0x33:
					gb3Pressed = true;
				break;
				case 0x34:
					gb4Pressed = true;
				break;
				case 0x35:
					gb5Pressed = true;
				break;
				case 0x36:
					gb6Pressed = true;
				break;
				case VK_ESCAPE:
					gbEscapePressed = true;
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
	void drawI(void);
	void drawN(void);
	void drawD(void);
	void drawA(void);
	void centerLines(void);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //GL_DEPTH_BUFFER_BIT added for 3D support
	//glColor3f(1.0f,0.0f,0.0f);
	//glColor3f(0.0f,1.0f,0.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gfFirstIAngle,0.0f,-3.0f);
	//glRotatef(0, 0.0f,0.0f,1.0f);
	drawI();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.5f,gfFirstNAngle,-3.0f);
	drawN();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	drawD();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.5f,gfSecondIAngle,-3.0f);
	drawI();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gfAAngle,0.0f,-3.0f);
	drawA();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gfCentralTransform,0.0f,-3.0f);
	//if(gfAAngle >= 1.0f){
	centerLines();
	//}
	
	SwapBuffers(ghdc);
}

void drawI(){
	glBegin(GL_LINES);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(0.0f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.0f,-0.75f,0.0f);
	glEnd();
}

void drawN(){
	glBegin(GL_LINES);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(-0.1f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(-0.1f,-0.75f,0.0f);
	glEnd();
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(-0.1f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.1f,-0.75f,0.0f);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(0.1f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.1f,-0.75f,0.0f);
	glEnd();
	glLineWidth(1.0f);
}

void drawD(){
	glBegin(GL_LINES);
	glColor3f(1.0f,gfDColor,0.0f);
	glVertex3f(-0.1f,0.75f,0.0f);
	glColor3f(0.0f,(GLfloat)(2*gfDColor),0.0f);
	glVertex3f(-0.1f,-0.75f,0.0f);
	glEnd();
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,gfDColor,0.0f);
	glVertex3f(0.1f,0.75f,0.0f);
	glColor3f(0.0f,(GLfloat)(2*gfDColor),0.0f);
	glVertex3f(0.1f,-0.75f,0.0f);
	glColor3f(1.0f,gfDColor,0.0f);
	glVertex3f(-0.15f,0.75f,0.0f);
	glVertex3f(0.10f,0.75f,0.0f);
	glColor3f(0.0f,(GLfloat)(2*gfDColor),0.0f);
	glVertex3f(-0.15f,-0.75f,0.0f);
	glVertex3f(0.10f,-0.75f,0.0f);
	glEnd();
	glLineWidth(1.0f);
}

void drawA(){
	//glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(0.0f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(-0.2f,-0.75f,0.0f);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(0.0f,0.75f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.2f,-0.75f,0.0f);
	glEnd();
}

void centerLines(){
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,0.5f,0.0f);
	glVertex3f(gfX1CentralLine,0.0f,0.0f);
	glVertex3f(gfX2CentralLine,0.0f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(gfX1CentralLine,-0.02f,0.0f);
	glVertex3f(gfX2CentralLine,-0.02f,0.0f);
	glEnd();
	
	glBegin(GL_LINES);
	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(gfX1CentralLine,-0.01f,0.0f);
	glVertex3f(gfX2CentralLine,-0.01f,0.0f);	
	glEnd();
	glLineWidth(1.0f);
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