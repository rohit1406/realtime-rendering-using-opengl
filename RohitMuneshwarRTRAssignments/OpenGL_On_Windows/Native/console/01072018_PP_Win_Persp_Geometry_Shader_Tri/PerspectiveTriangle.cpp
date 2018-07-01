#include<windows.h>
#include<stdio.h> //for file IO
#include<gl/glew.h> //for GLSL extensions
#include<gl/GL.h>

#include "vmath.h"

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
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
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
FILE *g_fp_logfile = NULL;

GLuint gVertexShaderObject;
GLuint gGeometryShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao;
GLuint gVbo;
GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

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
	
	if(fopen_s(&g_fp_logfile,"ppLog.txt","w")!=0)
	{
		MessageBox(NULL,TEXT("Log File Can Not Be Created\nExitting..."),TEXT("Error"),MB_OK | MB_TOPMOST | MB_ICONSTOP);
		uninitialize();
	}else
	{
		fprintf(g_fp_logfile,"\nLog file is opened successfully...\n");
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
	void resize(int,int);
	void uninitialize(void);
	
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
	
	//VERTEX SHADER
	//create shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	
	//provide source code to shader
	const GLchar *vertexShaderSourceCode =
				/*"#version 440" \
				"\n" \
				"in vec4 vPosition;" \
				"uniform mat4 u_mvp_matrix;" \
				"void main(void)" \
				"{" \
				"gl_Position=u_mvp_matrix * vPosition;" \
				"}";*/
				"#version 440							"\
				"\n										"\
				"in vec4 vPosition;						"\
				"uniform mat4 u_mvp_matrix;				"\
				"void main(void)						"\
				"{										"\
				"gl_Position = u_mvp_matrix * vPosition;	"\
				"}										";
				
				
	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode,NULL);
	fprintf(g_fp_logfile,"Actual shadig language version is %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObject);
	
	GLint iInfoLogLength=0;
	GLint iShaderCompileStatus=0;
	char *szInfoLog=NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	// *** GEOMETRY SHADER ***
	// create shader
	gGeometryShaderObject = glCreateShader(GL_GEOMETRY_SHADER);

	// provide source code to shader
	const GLchar *geometryShaderSourceCode =
		"#version 410" \
		"\n" \
		"layout(triangles)in;"\
		"layout(triangle_strip, max_vertices = 9)out;"\
		"uniform mat4 u_mvp_matrix;"\
		"void main(void)"\
		"{"\
		"for(int vertex=0; vertex<3; vertex++)"\
		"{"\
		"gl_Position= u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));"\
		"EmitVertex();"\
		"gl_Position= u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(-1.0, -1.0, 0.0, 0.0));"\
		"EmitVertex();"\
		"gl_Position= u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(1.0, -1.0, 0.0, 0.0));"\
		"EmitVertex();"\
		"EndPrimitive();"\
		"}"\
		"}";
	glShaderSource(gGeometryShaderObject, 1, (const GLchar **)&geometryShaderSourceCode, NULL);

	// compile shader
	glCompileShader(gGeometryShaderObject);
	// re-initialize
	iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	szInfoLog = NULL;
	glGetShaderiv(gGeometryShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gGeometryShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gGeometryShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Geometry Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *fragmentShaderSourceCode =
				"#version 440" \
				"\n" \
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(1.0,1.0,1.0,1.0);" \
				"}";
				
	glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode,NULL);
	
	//compile shader
	glCompileShader(gFragmentShaderObject);
	//reinitialize 
	 iInfoLogLength = 0;
	 iShaderCompileStatus = 0;
	 szInfoLog = NULL;
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile,"Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//SHADER PROGRAM
	//create
	gShaderProgramObject = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObject,gVertexShaderObject);

	// attach geometry shader to shader program
	glAttachShader(gShaderProgramObject, gGeometryShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	// pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");

	// link shader
	glLinkProgram(gShaderProgramObject);
	GLint iShaderProgramLinkStatus=0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//get MVP uniform location
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	
	//vertices, colors, shader attribs, vbo, vao initializations
	const GLfloat triangleVertices[]=
	{
		0.0f,1.0f,0.0f, //apex
		-1.0f,-1.0f,0.0f, // lb
		1.0f,-1.0f,0.0f //rb
	};
	
	glGenVertexArrays(1, &gVao);
	glBindVertexArray(gVao);
	
	glGenBuffers(1, &gVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
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
	glEnable(GL_CULL_FACE);
	
	glClearColor(0.1f,0.2f,0.3f,0.0f);
	
	//set orthographicMatrix to identify matrix
	gPerspectiveProjectionMatrix = mat4::identity();
	
	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // | GL_STENCIL_BUFFER add kelyawar output disat nhi.... why?
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix=mat4::identity();
	
	//translate
	modelViewMatrix = translate(0.0f,0.0f,-4.0f);
	
	//multiply the modelview and perspective matrix to get modelviewprojection matrix
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //order is important
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	
	//bind vao
	glBindVertexArray(gVao);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, 0,3);
	
	//unbind vao
	glBindVertexArray(0);
	
	//stop using OpenGL program object
	glUseProgram(0);
	
	SwapBuffers(ghdc);
}

void resize(int width,int height){
	if(height == 0){
		height = 1;
	}
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	
	//glOrtho(left,right,bottom,top,near,far);
	/*if(width<=height)
	{
			gOrthographicProjectionMatrix = ortho(-100.0f,100.0f, (-100.0f * (height/width)), (100.0f * (height/width)), -100.0f, 100.0f);
	}else
	{
		gOrthographicProjectionMatrix = ortho(-100.0f, 100.0f, (-100.0f * (width/height)), (100.0f * (width/height)), -100.0f, 100.0f);
	}*/
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
	if(gVao)
	{
		glDeleteVertexArrays(1, &gVao);
		gVao = 0;
	}
	
	//destroy vbo
	if(gVbo)
	{
		glDeleteBuffers(1, &gVbo);
		gVbo = 0;
	}
	
	//detach vertex shader from shader program object
	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	// detach geometry shader from shader program object
	glDetachShader(gShaderProgramObject, gGeometryShaderObject);
	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);
	
	//delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	
	//delete fragment shader object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject=0;
	// delete geometry shader object
	glDeleteShader(gGeometryShaderObject);
	gGeometryShaderObject = 0;
	//delete shader program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject=0;
	
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
	
	if(g_fp_logfile){
		fprintf(g_fp_logfile,"\nLog file is closed successfully...\n");
		fclose(g_fp_logfile);
		g_fp_logfile = NULL;
	}
	DestroyWindow(ghwnd);
	ghwnd = NULL;
}