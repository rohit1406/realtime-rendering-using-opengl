#include<windows.h>
#include<stdio.h> //for file IO
#include<gl/glew.h> //for GLSL extensions
#include<gl/GL.h>

#include "vmath.h"

//to load image into memory
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX = 0,
	VDG_ATTRIBUTE_COLOR =1,
	VDG_ATTRIBUTE_NORMAL =2,
	VDG_ATTRIBUTE_TEXTURE0 =3
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

bool gbDone = false;
bool gbFullScreen = false;
HWND ghwnd=NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)}; //for fullscreen

//changes for opengl
bool gbActiveWindow = false;
bool gbEscapePressed = false;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

//log file
FILE *gLogFile = NULL;

//shaders
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

//to render from lights point of view
GLuint gVertexShaderObjectLightsView;
GLuint gFragmentShaderObjectLightsView;
GLuint gShaderProgramObjectLightsView;

//for quad
GLuint gVertexShaderObjectQuad;
GLuint gFragmentShaderObjectQuad;
GLuint gShaderProgramObjectQuad;

//frame buffer object
GLuint gFboDepthMap;

GLuint gTextureDepthMap;

//vao and vbo
GLuint gVaoPlane;
GLuint gVboPlane;
GLuint gVaoCube;
GLuint gVboCube;
GLuint gVaoQuad;
GLuint gVboQuad;

//uniform variables
GLuint gUniformModel, gUniformView, gUniformProjection;
GLuint gUniformTextureBricksSampler; //for bricks texture
GLuint gUniformTextureDepthMapSampler; //for bricks texture

//from lights point of view
GLuint gUniformModelLightsView, gUniformViewLightsView, gUniformProjectionLightsView;

//for quad
GLuint gUniformModelQuad, gUniformViewQuad, gUniformProjectionQuad;

//textures
GLuint gTextureBricks;

//perspective projection matrix
mat4 gPerspectiveProjectionMatrix;

//orthographic projection matrix
mat4 gOrthographicProjectionMatrix;

//set modelview & modelviewprojection matrices to identity
mat4 gTransalationTransformationMatrix;
mat4 gRotationTransformationMatrix;
mat4 gScaleTransformationMatrix;
mat4 gModelTransformationMatrix;
mat4 gViewTransformationMatrix;
mat4 gProjectionTransformationMatrix;

vec3 gLightColor = vec3(1.0f, 1.0f, 1.0f); //light color
vec3 gLightPosition = vec3(-2.0f, 4.0f, -4.0f); //light position
vec3 gCameraPosition = vec3(0.0f, 0.0f, 3.0f); //camera/viewer position

GLint SHADOW_WIDTH = 1024;
GLint SHADOW_HEIGHT = 1024;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLIne, int iCmdShow){
	void initialize(void);
	void uninitialize(void);
	void display(void);
	WNDCLASSEX wndclass;
	TCHAR AppName[] = TEXT("Window Custom");
	TCHAR WinName[] = TEXT("Perspective Triangle using Programmable Pipeline");
	HWND hwnd;
	MSG msg;
	RECT rect;
	
	if(fopen_s(&gLogFile,"ppLog.txt","w")!=0)
	{
		MessageBox(NULL,TEXT("Log File Can Not Be Created\nExitting..."),TEXT("Error"),MB_OK | MB_TOPMOST | MB_ICONSTOP);
		uninitialize();
	}else
	{
		fprintf(gLogFile,"\nLog file is opened successfully...\n");
	}
	
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
	
	uninitialize();
	return ((int) msg.wParam);
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
					if(gbEscapePressed==false)
						gbEscapePressed = true;
					break;
				break;
				case 0x46: //F or f key
					if(gbFullScreen==false)
					{
						ToggleFullscreen();
						gbFullScreen = true;
					}else
					{
						ToggleFullscreen();
						gbFullScreen = false;
					}
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
	//function prototype declaration
	void createVertexShader(void);
	void createFragmentShader(void);
	void createShaderProgram(void);
	void getUniformLocations(void);
	void initializeVaos(void);
	void initializeGLProperties(void);
	void generateDepthTexture(void);
	void loadTexture(GLuint *, char *);
	void resize(int,int);
	void uninitialize(void);

	//code
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	
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
	pfd.cDepthBits=32;
	
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
	
	//GLEW initialization code for GLSL
	GLenum glew_error = glewInit();
	if(glew_error != GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	
	createVertexShader(); //create vertex shader
	createFragmentShader(); //create fragment shader
	createShaderProgram(); //create shader program
	getUniformLocations(); //get uniform locations

	//configure frame buffer objects
	glGenFramebuffers(1, //how many frame buffer objects you want to create
		&gFboDepthMap //fbo object
	);

	//bind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, gFboDepthMap);
	
	generateDepthTexture();

	//attach above generated texture as the framebuffers depth buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, //framebuffer target
		GL_DEPTH_ATTACHMENT, //attach as a depth buffer
		GL_TEXTURE_2D, //texture attachment target
		gTextureDepthMap, //depth buffer
		0);

	//tell opengl that we are not going to render any color data
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	initializeVaos(); //initialize vaos
	initializeGLProperties(); //set OpenGL states
	
	//load texture data
	loadTexture(&gTextureBricks, "bricks.png");

	//set transformation matrices to identify matrix
	gTransalationTransformationMatrix = mat4::identity();
	gRotationTransformationMatrix = mat4::identity();
	gScaleTransformationMatrix = mat4::identity();
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	gProjectionTransformationMatrix = mat4::identity();
	gOrthographicProjectionMatrix = mat4::identity();


	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void display(){
	//function prototypes
	void drawCube(void);
	void drawPlane(void);
	void drawQuad(void);
	//code
	//set transformation matrices to identify matrix
	gTransalationTransformationMatrix = mat4::identity();
	gRotationTransformationMatrix = mat4::identity();
	gScaleTransformationMatrix = mat4::identity();
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	//gProjectionTransformationMatrix = mat4::identity();
	//gOrthographicProjectionMatrix = mat4::identity();

	//1. first render to depthmap
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, gFboDepthMap);
	//bind texture
	glBindTexture(GL_TEXTURE_2D, //texture target
		gTextureDepthMap //texture object to bind
	);
	glClear(GL_DEPTH_BUFFER_BIT);

	//use program object
	glUseProgram(gShaderProgramObjectLightsView);
	//model transformation
	//translate
	gTransalationTransformationMatrix = translate(0.0f, 1.5f, -4.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//configure shaders and matrices
	//view transformation
	//creating a view matrix to transform each object so that they are visible from
	//the lights point of view
	gViewTransformationMatrix = lookat(gLightPosition,//position of camera
		vec3(0.0f, 0.0f, 0.0f), //target - where camera is looking at
		vec3(0.0f, 1.0f, 0.0f)//what is up vector
	);

	//projection transformation
	gProjectionTransformationMatrix = gOrthographicProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModelLightsView, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformViewLightsView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjectionLightsView, 1, GL_FALSE, gProjectionTransformationMatrix);

	//render scene
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	//glUniform1i(gUniformTextureBricksSampler, 0);
	drawPlane();

	//draw cubes
	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	//gViewTransformationMatrix = mat4::identity();
	//gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(0.0f, 1.5f, -4.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.5f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	//gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	//glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();

	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	//gViewTransformationMatrix = mat4::identity();
	//gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(2.0f, 0.0f, -5.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.5f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	//gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	//glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();


	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	//gViewTransformationMatrix = mat4::identity();
	//gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(-1.0f, 0.0f, -4.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//rotate
	gRotationTransformationMatrix = rotate(radians(60.0f), 1.0f, 0.0f, 1.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gRotationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.25f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	//gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	//glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();
	//unbind vao
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, //texture target
		0 //texture object to bind
	);

	//unbind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//restore viewport dimentions
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
	//rendering to depthmap ends

	
	//2. render scene as normal with shadow mapping using depth map
	glClear(GL_COLOR_BUFFER_BIT //clear color buffer
				| GL_DEPTH_BUFFER_BIT //clear depth buffer as we are using depth testing
			); // | GL_STENCIL_BUFFER
	
	//render quad
	glUseProgram(gShaderProgramObjectQuad);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModelLightsView, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformViewLightsView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjectionLightsView, 1, GL_FALSE, gProjectionTransformationMatrix);

	//bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureDepthMap);
	glUniform1i(gUniformTextureDepthMapSampler, 0);

	//draw quad
	drawQuad();
	//unbind program
	glUseProgram(0);
	/*
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	gProjectionTransformationMatrix=mat4::identity();
	
	//translate
	gTransalationTransformationMatrix = translate(0.0f, 0.0f, -3.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//set projection matrix
	gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	glUniform1i(gUniformTextureBricksSampler, 0);
	drawPlane();

	//draw cubes
	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(0.0f, 1.5f, -6.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.5f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();

	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(2.0f, 0.0f, -5.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.5f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();


	//set modelview & modelviewprojection matrices to identity
	gModelTransformationMatrix = mat4::identity();
	gViewTransformationMatrix = mat4::identity();
	gProjectionTransformationMatrix = mat4::identity();

	//translate
	gTransalationTransformationMatrix = translate(-1.0f, 0.0f, -4.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gTransalationTransformationMatrix;

	//rotate
	gRotationTransformationMatrix = rotate(radians(60.0f), 1.0f, 0.0f, 1.0f);
	gModelTransformationMatrix = gModelTransformationMatrix * gRotationTransformationMatrix;

	//scale
	gScaleTransformationMatrix = vmath::scale(0.25f);
	gModelTransformationMatrix = gModelTransformationMatrix * gScaleTransformationMatrix;

	//set projection matrix
	gProjectionTransformationMatrix = gPerspectiveProjectionMatrix;

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gUniformModel, 1, GL_FALSE, gModelTransformationMatrix);
	glUniformMatrix4fv(gUniformView, 1, GL_FALSE, gViewTransformationMatrix);
	glUniformMatrix4fv(gUniformProjection, 1, GL_FALSE, gProjectionTransformationMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBricks);
	glUniform1i(gUniformTextureBricksSampler, 0);
	drawCube();
	
	//stop using OpenGL program object
	glUseProgram(0);
	*/
	
	SwapBuffers(ghdc);
}

void resize(int width,int height){
	if(height == 0){
		height = 1;
	}
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	
	//for capturing texture coords from lights perspective
	//glOrtho(left,right,bottom,top,near,far);
	if (width <= height)
	{
		gOrthographicProjectionMatrix = ortho(-100.0f, 100.0f, (-100.0f * (height / width)), (100.0f * (height / width)), -100.0f, 100.0f);
	}
	else
	{
		gOrthographicProjectionMatrix = ortho(-100.0f, 100.0f, (-100.0f * (width / height)), (100.0f * (width / height)), -100.0f, 100.0f);
	}

	//for rendering from viewer perspective
	gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f,100.0f);
}

void uninitialize(){
	if(gbFullScreen == true){
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd,GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
	
	//destroy vao
	if(gVaoPlane)
	{
		glDeleteVertexArrays(1, &gVaoPlane);
		gVaoPlane = 0;
	}

	if (gVaoCube)
	{
		glDeleteVertexArrays(1, &gVaoCube);
		gVaoCube = 0;
	}
	
	//destroy vbo
	if(gVboCube)
	{
		glDeleteBuffers(1, &gVboCube);
		gVboCube = 0;
	}

	//destroy texture
	if (gTextureBricks)
	{
		glDeleteTextures(1, &gTextureBricks);
		gTextureBricks = 0;
	}
	
	//detach vertex shader from shader program object
	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	
	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);
	
	//delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	
	//delete fragment shader object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject=0;
	
	//delete shader program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject=0;

	//detach vertex shader from shader program object
	glDetachShader(gShaderProgramObjectLightsView, gVertexShaderObjectLightsView);

	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObjectLightsView, gFragmentShaderObjectLightsView);

	//delete vertex shader object
	glDeleteShader(gVertexShaderObjectLightsView);
	gVertexShaderObjectLightsView = 0;

	//delete fragment shader object
	glDeleteShader(gFragmentShaderObjectLightsView);
	gFragmentShaderObjectLightsView = 0;

	//delete shader program object
	glDeleteProgram(gShaderProgramObjectLightsView);
	gShaderProgramObjectLightsView = 0;
	

	//unlink shader program
	glUseProgram(0);
	
	//deselect the rendering context
	wglMakeCurrent(NULL,NULL);
	
	//delete the rendering context
	wglDeleteContext(ghrc);
	ghrc = NULL;
	
	//delete the device context
	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;
	
	if(gLogFile){
		fprintf(gLogFile,"\nLog file is closed successfully...\n");
		fclose(gLogFile);
		gLogFile = NULL;
	}
	DestroyWindow(ghwnd);
	ghwnd = NULL;
}

void createVertexShader(void)
{
	fprintf(gLogFile, "creating vertex shader initiated...\n");
	//function prototype declaration
	void uninitialize(void);

	//local variables declaration
	GLint iInfoLogLength = 0;
	GLint iShaderCompileStatus = 0;
	char *szInfoLog = NULL;

	//code
	//VERTEX SHADER
	//create shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//provide source code to shader
	const GLchar *vertexShaderSourceCode =
		"#version 440							"\
		"\n										"\
		"in vec3 vPosition;"\
		"in vec3 vNormal;"\
		"in vec2 vTexCoords;"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"out vec2 out_tex_coords;"\
		"void main(void)						"\
		"{										"\
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"out_tex_coords = vTexCoords;"\
		"}										";


	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
	fprintf(gLogFile, "Actual shadig language version is %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObject);

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//VERTEX SHADER for lights point of view
	//create shader
	gVertexShaderObjectLightsView = glCreateShader(GL_VERTEX_SHADER);

	//provide source code to shader
	const GLchar *vertexShaderSourceCodeLightsView =
		"#version 440							"\
		"\n										"\
		"in vec3 vPosition;"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"void main(void)						"\
		"{										"\
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"}										";


	glShaderSource(gVertexShaderObjectLightsView, 1, (const GLchar**)&vertexShaderSourceCodeLightsView, NULL);
	fprintf(gLogFile, "Actual shadig language version is %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObjectLightsView);

	glGetShaderiv(gVertexShaderObjectLightsView, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectLightsView, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectLightsView, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//VERTEX SHADER for quad
	//create shader
	gVertexShaderObjectQuad = glCreateShader(GL_VERTEX_SHADER);

	//provide source code to shader
	const GLchar *vertexShaderSourceCodeQuad =
		"#version 440							"\
		"\n										"\
		"in vec3 vPosition;"\
		"in vec2 vTexCoords;"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"out vec2 out_tex_coords;"\
		"void main(void)						"\
		"{										"\
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"out_tex_coords = vTexCoords;"\
		"}										";


	glShaderSource(gVertexShaderObjectQuad, 1, (const GLchar**)&vertexShaderSourceCodeQuad, NULL);
	fprintf(gLogFile, "Actual shadig language version is %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObjectQuad);

	glGetShaderiv(gVertexShaderObjectQuad, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectQuad, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectQuad, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	fprintf(gLogFile, "vertex shader created...\n");
}

void createFragmentShader(void)
{
	fprintf(gLogFile, "creating fragment shader initiated...\n");
	//function prototype declaration
	void uninitialize(void);

	//local variables declaration
	GLint iInfoLogLength = 0;
	GLint iShaderCompileStatus = 0;
	char *szInfoLog = NULL;

	//code
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//provide source code to shader
	const GLchar *fragmentShaderSourceCode =
		"#version 440" \
		"\n" \
		"in vec2 out_tex_coords;"\
		"uniform sampler2D u_texture_bricks_sampler;"\
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor=texture(u_texture_bricks_sampler, out_tex_coords);" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	//compile shader
	glCompileShader(gFragmentShaderObject);
	
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}



	//FRAGMENT SHADER from lights point of view
	//create shader
	gFragmentShaderObjectLightsView = glCreateShader(GL_FRAGMENT_SHADER);

	//provide source code to shader
	const GLchar *fragmentShaderSourceCodeLightsView =
		"#version 440" \
		"\n" \
		"void main(void)" \
		"{" \
		"}";

	glShaderSource(gFragmentShaderObjectLightsView, 1, (const GLchar**)&fragmentShaderSourceCodeLightsView, NULL);

	//compile shader
	glCompileShader(gFragmentShaderObjectLightsView);

	glGetShaderiv(gFragmentShaderObjectLightsView, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectLightsView, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectLightsView, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//for quad
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObjectQuad = glCreateShader(GL_FRAGMENT_SHADER);

	//provide source code to shader
	const GLchar *fragmentShaderSourceCodeQuad =
		"#version 440" \
		"\n" \
		"in vec2 out_tex_coords;"\
		"uniform sampler2D u_texture_depth_map_sampler;"\
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"float depthValue = texture(u_texture_depth_map_sampler, out_tex_coords).r;"\
		"FragColor=vec4(vec3(depthValue), 1.0);" \
		"}";

	glShaderSource(gFragmentShaderObjectQuad, 1, (const GLchar**)&fragmentShaderSourceCodeQuad, NULL);

	//compile shader
	glCompileShader(gFragmentShaderObjectQuad);

	glGetShaderiv(gFragmentShaderObjectQuad, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectQuad, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectQuad, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	fprintf(gLogFile, "fragment shader created...\n");
}

void createShaderProgram(void)
{
	fprintf(gLogFile, "creating shader program initiated...\n");
	//function prototype declaration
	void uninitialize(void);

	//local variables declaration
	GLint iShaderProgramLinkStatus = 0;
	GLint iInfoLogLength = 0;
	char *szInfoLog = NULL;

	//code
	//SHADER PROGRAM
	//create
	gShaderProgramObject = glCreateProgram();

	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0, "vTexCoords");

	//link shader
	glLinkProgram(gShaderProgramObject);
	
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//SHADER PROGRAM from lights point of view
	//create
	gShaderProgramObjectLightsView = glCreateProgram();

	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObjectLightsView, gVertexShaderObjectLightsView);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObjectLightsView, gFragmentShaderObjectLightsView);

	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObjectLightsView, VDG_ATTRIBUTE_VERTEX, "vPosition");

	//link shader
	glLinkProgram(gShaderProgramObjectLightsView);

	glGetProgramiv(gShaderProgramObjectLightsView, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectLightsView, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectLightsView, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//for quad
	//SHADER PROGRAM
	//create
	gShaderProgramObjectQuad = glCreateProgram();

	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObjectQuad, gVertexShaderObjectQuad);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObjectQuad, gFragmentShaderObjectQuad);

	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObjectQuad, VDG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0, "vTexCoords");

	//link shader
	glLinkProgram(gShaderProgramObjectQuad);

	glGetProgramiv(gShaderProgramObjectQuad, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectQuad, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectQuad, iInfoLogLength, &written, szInfoLog);
				fprintf(gLogFile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	fprintf(gLogFile, "shader program created...\n");
}

void getUniformLocations(void)
{
	fprintf(gLogFile, "getting uniform locations...\n");
	//get MVP uniform location
	gUniformModel = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gUniformView = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gUniformProjection = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gUniformTextureBricksSampler = glGetUniformLocation(gShaderProgramObject, "u_texture_bricks_sampler");

	gUniformModelLightsView = glGetUniformLocation(gShaderProgramObjectLightsView, "u_model_matrix");
	gUniformViewLightsView = glGetUniformLocation(gShaderProgramObjectLightsView, "u_view_matrix");
	gUniformProjectionLightsView = glGetUniformLocation(gShaderProgramObjectLightsView, "u_projection_matrix");

	//for depth map
	gUniformModelQuad = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gUniformViewQuad = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gUniformProjectionQuad = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gUniformTextureDepthMapSampler = glGetUniformLocation(gShaderProgramObject, "u_texture_depth_map_sampler");
}

void initializeVaos(void)
{
	fprintf(gLogFile, "configuring vaos...\n");
	//vertices, colors, shader attribs, vbo, vao initializations
	//for element buffer - see the data is created in anticlock-wise direction
	const GLfloat vertexData[] =
	{
		//position			   //normals         //texcoords
		-25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		25.0f, -0.5f, 25.0f,   0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		
		25.0f, -0.5f, 25.0f,   0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,  0.0f, 25.0f
	};

	float cubeVertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	//vao
	glGenVertexArrays(1, &gVaoPlane);
	glBindVertexArray(gVaoPlane);

	glGenBuffers(1, &gVboPlane);

	//from this point onwards, any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to configure the currently bound buffer, which is gVbo
	glBindBuffer(GL_ARRAY_BUFFER, //out of many types of buffer objects, the buffer type of vertex buffer object (vbo) is GL_ARRAY_BUFFER
		gVboPlane //buffer object to bind
	);

	//copies previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, //type of the buffer we want to copy data into
		sizeof(vertexData), //size of the data in bytes which we want to pass to the buffer
		vertexData, //actual data we want to send
		GL_STATIC_DRAW //how we want graphics card to manage the given data
	);

	//tell OpenGL how it should interpret the vertex data (per vertex attribute)
	//position of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, //location of vertex attribute - which vertex attribute you want to configure
		3, //size of vertex attribute
		GL_FLOAT, //type of the data in vertex attribute
		GL_FALSE, // if we want data to be normalized
		8 * sizeof(float), // stride - the space between consecutive vertex attribute sets
		(void *)0 // offset of where the position data begins in the buffer
	);

	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

	//normal data of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, //location of texture coords attribute
		3, //number of points used to represent texel in the vertex attributes
		GL_FLOAT, //type of the data in the texture attribute
		GL_FALSE, //if you want to normalize
		8 * sizeof(GL_FLOAT), //stride - the space between consecutive vertex attributes set
		(void *)(3 * sizeof(GL_FLOAT)) //offset of where the texture coords begins in the buffer
	);

	//enable texture attribute by giving the texture attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);

	//texture data of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, //location of texture coords attribute
		2, //number of points used to represent texel in the vertex attributes
		GL_FLOAT, //type of the data in the texture attribute
		GL_FALSE, //if you want to normalize
		8 * sizeof(GL_FLOAT), //stride - the space between consecutive vertex attributes set
		(void *) (2 * 3 * sizeof(GL_FLOAT)) //offset of where the texture coords begins in the buffer
	);

	//enable texture attribute by giving the texture attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	//configure cube vaos and vbos
	//vao
	glGenVertexArrays(1, &gVaoCube);
	glBindVertexArray(gVaoCube);

	glGenBuffers(1, &gVboCube);

	//from this point onwards, any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to configure the currently bound buffer, which is gVbo
	glBindBuffer(GL_ARRAY_BUFFER, //out of many types of buffer objects, the buffer type of vertex buffer object (vbo) is GL_ARRAY_BUFFER
		gVboCube //buffer object to bind
	);

	//copies previously defined vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, //type of the buffer we want to copy data into
		sizeof(cubeVertices), //size of the data in bytes which we want to pass to the buffer
		cubeVertices, //actual data we want to send
		GL_STATIC_DRAW //how we want graphics card to manage the given data
	);

	//tell OpenGL how it should interpret the vertex data (per vertex attribute)
	//position of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, //location of vertex attribute - which vertex attribute you want to configure
		3, //size of vertex attribute
		GL_FLOAT, //type of the data in vertex attribute
		GL_FALSE, // if we want data to be normalized
		8 * sizeof(float), // stride - the space between consecutive vertex attribute sets
		(void *)0 // offset of where the position data begins in the buffer
	);

	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

	//normal data of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, //location of texture coords attribute
		3, //number of points used to represent texel in the vertex attributes
		GL_FLOAT, //type of the data in the texture attribute
		GL_FALSE, //if you want to normalize
		8 * sizeof(GL_FLOAT), //stride - the space between consecutive vertex attributes set
		(void *)(3 * sizeof(GL_FLOAT)) //offset of where the texture coords begins in the buffer
	);

	//enable texture attribute by giving the texture attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);

	//texture data of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, //location of texture coords attribute
		2, //number of points used to represent texel in the vertex attributes
		GL_FLOAT, //type of the data in the texture attribute
		GL_FALSE, //if you want to normalize
		8 * sizeof(GL_FLOAT), //stride - the space between consecutive vertex attributes set
		(void *)(2 * 3 * sizeof(GL_FLOAT)) //offset of where the texture coords begins in the buffer
	);

	//enable texture attribute by giving the texture attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	//configure quad vaos and vbos
	float quadVertices[] =
	{
		//position			//texture coords
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f
	};

	//setup vao
	//generate vao
	glGenVertexArrays(1, //how many
	&gVaoQuad //address of vao
	);

	//bind vao
	glBindVertexArray(gVaoQuad);

	//generate buffer
	glGenBuffers(1, //how many
		&gVboQuad //address of vbo
		);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, gVboQuad);

	//add data to buffer
	glBufferData(GL_ARRAY_BUFFER, //in what type of buffer you want to add data
		sizeof(quadVertices), //size of data you want to add
		quadVertices, //actual data
		GL_STATIC_DRAW //how you want to maintain data in buffer
		);

	//how you want to manage data in the buffer
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, //location of vertex data in shader
		3, //points representing one vertex attribute
		GL_FLOAT, //data type of points
		GL_FALSE, //no need to normalize data
		5 * sizeof(GL_FLOAT), //stride
		(void *) 0 //offset of data starting point in buffer
	);

	//enable above attib location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

	//how you want to manage data in the buffer
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, //location of texture data in shader
		2, //points representing one vertex attribute
		GL_FLOAT, //data type of points
		GL_FALSE, //no need to normalize data
		5 * sizeof(GL_FLOAT), //stride
		(void *)(3 * sizeof(GL_FLOAT)) //offset of data starting point in buffer
	);

	//enable above attib location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//unbind vao
	glBindVertexArray(0);
	//configure quad vaos and vbos ends
	fprintf(gLogFile, "vaos configured...\n");
}

void initializeGLProperties(void)
{
	fprintf(gLogFile, "configuring OpenGL properties...\n");
	glShadeModel(GL_SMOOTH);

	//set-up depth buffer
	glClearDepth(1.0f);

	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	//depth test to do
	glDepthFunc(GL_LEQUAL);

	//set really ice perspective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//we will always cull back faces for better performance
	//glEnable(GL_CULL_FACE);

	//wireframe model
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void drawCube(void)
{

	//bind vao
	glBindVertexArray(gVaoCube);

	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
		0, //starting index of the vertex array we'd like to draw
		36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
	);

	//unbind vao
	glBindVertexArray(0);
}

void drawPlane(void)
{
	//bind vao
	glBindVertexArray(gVaoPlane);

	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
		0, //starting index of the vertex array we'd like to draw
		6 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
	);

	//unbind vao
	glBindVertexArray(0);
}

//send address of texture object and image file path to this function to load the image
//into texture object
void loadTexture(GLuint *texture, char* path)
{
	//generate texture
	glGenTextures(1, //how many texture objects
		texture //address of texture object to generate - texture gets stored in this
	);

	//bind texture to make it active
	glBindTexture(GL_TEXTURE_2D, //what type of texture you want to bind
		*texture//texture to bind
	);

	//load image data to memory
	int width, height, nrChannels;
	GLubyte* imageData = stbi_load(path, //path of image to load
		&width, //stores image width
		&height, //stores image height
		&nrChannels, //number of colored channels
		0
	);

	if (imageData)
	{
		//to check the image format
		GLenum format;
		if (nrChannels==1)
		{
			format = GL_RED;
		}else if (nrChannels == 3)
		{
			format = GL_RGB;
		}else if (nrChannels == 4)
		{
			format = GL_RGBA;
		}

		glTexImage2D(GL_TEXTURE_2D, //what type of texture you want to load
			0, //mipmap level for which you want to create texture for
			format, //in what kind of format you want to store texture
			width, //width of resulting texture
			height, //height of resulting texture
			0, //should always be 0 - some legacy stuff
			format, //format of source image
			GL_UNSIGNED_BYTE, //datatype of source image
			imageData //actual image data
		);

		//currently bound texture object now has base-level of texture image
		//attached to it

		//generate all the required mipmaps for the currently bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		fprintf(gLogFile, "Failed to load the image data...");
	}

	//free image memory
	stbi_image_free(imageData);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void generateDepthTexture(void)
{
	//code
	//generate texture
	glGenTextures(1, //how many texture objects
		&gTextureDepthMap //texture object
	);

	//bind texture
	glBindTexture(GL_TEXTURE_2D, //texture target
		gTextureDepthMap //texture object to bind
	);

	//initialize
	glTexImage2D(GL_TEXTURE_2D,
		0, //mipmap level
		GL_DEPTH_COMPONENT, //internal format
		SHADOW_WIDTH,
		SHADOW_HEIGHT,
		0,
		GL_DEPTH_COMPONENT, //format
		GL_FLOAT, //datatype of image data
		NULL //passing data as null
	);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, //texture target
		0 //texture object to bind
	);
}

void drawQuad(void)
{

	//bind vao
	glBindVertexArray(gVaoQuad);

	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLE_STRIP, //primitive type we like to draw
		0, //starting index of the vertex array we'd like to draw
		4 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
	);

	//unbind vao
	glBindVertexArray(0);
}