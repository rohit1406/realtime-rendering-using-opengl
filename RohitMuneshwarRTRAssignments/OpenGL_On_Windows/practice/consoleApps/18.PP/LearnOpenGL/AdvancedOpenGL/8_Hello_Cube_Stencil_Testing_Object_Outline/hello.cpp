#include<windows.h>
#include<stdio.h> //for file IO
#include<gl/glew.h> //for GLSL extensions
#include<gl/GL.h>

#include "vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
FILE *g_fp_logfile = NULL;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

//for outline
GLuint gOutlineFragmentShaderObject;
GLuint gSingleColorShaderProgramObject;

GLuint gVao;
GLuint gVbo;
GLuint gEbo;
GLuint gVbo_col;

GLuint gVaoFloor;
GLuint gVboFloor;

GLuint gModelUniform, gViewUniform, gProjectionUniform;
GLuint gSamplerUniform;
GLuint container_texture;

//floor texture
GLuint gFloorTexture;

GLfloat rotationAngle=0.0f;
mat4 gPerspectiveProjectionMatrix;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLIne, int iCmdShow){
	void initialize(void);
	void uninitialize(void);
	void display(void);
	void updateAngle(void);
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
				updateAngle();
				display(); //for double buffer
			}
		}
	}
	
	uninitialize();
	return ((int) msg.wParam);
}

void updateAngle()
{
	rotationAngle = rotationAngle+0.01f;
	if(rotationAngle>=360.0f)
	{
		rotationAngle=rotationAngle-360.0f;
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
	void loadTexture(GLuint *, char* path);
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
				"#version 440							"\
				"\n										"\
				"in vec3 vPosition;						"\
				"in vec3 vColor;"\
				"in vec2 vTexCoords;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"out vec3 out_color;"\
				"out vec2 out_tex_coords;"\
				"void main(void)						"\
				"{										"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
				"out_color=vColor;"\
				"out_tex_coords=vTexCoords;"\
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
	
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *fragmentShaderSourceCode =
				"#version 440" \
				"\n" \
				"in vec3 out_color;"\
				"in vec2 out_tex_coords;"\
				"uniform sampler2D u_sampler_texture;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=texture(u_sampler_texture, out_tex_coords);" \
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

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObject,gFragmentShaderObject);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_COLOR,"vColor");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0,"vTexCoords");
	
	//link shader
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
	
	
	//FRAGMENT SHADER For Outline
	//create shader
	gOutlineFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *outlineFragmentShaderSourceCode =
				"#version 440" \
				"\n" \
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(0.04, 0.28, 0.26, 1.0);" \
				"}";
				
	glShaderSource(gOutlineFragmentShaderObject, 1, (const GLchar**)&outlineFragmentShaderSourceCode,NULL);
	
	//compile shader
	glCompileShader(gOutlineFragmentShaderObject);
	//reinitialize 
	 iInfoLogLength = 0;
	 iShaderCompileStatus = 0;
	 szInfoLog = NULL;
	glGetShaderiv(gOutlineFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gOutlineFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gOutlineFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile,"Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//Single Color SHADER PROGRAM
	//create
	gSingleColorShaderProgramObject = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gSingleColorShaderProgramObject,gVertexShaderObject);

	//attach fragment shader to shader program
	glAttachShader(gSingleColorShaderProgramObject,gOutlineFragmentShaderObject);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gSingleColorShaderProgramObject, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gSingleColorShaderProgramObject, VDG_ATTRIBUTE_COLOR,"vColor");
	glBindAttribLocation(gSingleColorShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0,"vTexCoords");
	
	//link shader
	glLinkProgram(gSingleColorShaderProgramObject);
	iShaderProgramLinkStatus=0;
	glGetProgramiv(gSingleColorShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gSingleColorShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gSingleColorShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	} 
	//single color shader program object creation ended here
	
	//get MVP uniform location
	gModelUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gSamplerUniform = glGetUniformLocation(gShaderProgramObject, "u_sampler_texture");
	
	//for single color shader program object
	/*gModelUniform = glGetUniformLocation(gSingleColorShaderProgramObject, "u_model_matrix");
	gViewUniform = glGetUniformLocation(gSingleColorShaderProgramObject, "u_view_matrix");
	gProjectionUniform = glGetUniformLocation(gSingleColorShaderProgramObject, "u_projection_matrix");*/
	
	//vertices, colors, shader attribs, vbo, vao initializations
	//for element buffer - see the data is created in anticlock-wise direction
	const GLfloat vertexData[]=
	{
	//position			  //texture
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	
	
	//floor vertieces
	float floorVertices[]=
	{
		//position			//texture coordinates
		5.0f, -0.5f, 5.0f,	2.0f, 0.0f,
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,0.0f, 2.0f,
		
		5.0f, -0.5f, 5.0f, 	2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,0.0f, 2.0f,
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f
	};
	
	//vao
	glGenVertexArrays(1, &gVao);
	glBindVertexArray(gVao);
	
	glGenBuffers(1, &gVbo);
	
	//from this point onwards, any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to configure the currently bound buffer, which is gVbo
	glBindBuffer(GL_ARRAY_BUFFER, //out of many types of buffer objects, the buffer type of vertex buffer object (vbo) is GL_ARRAY_BUFFER
				gVbo //buffer object to bind
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
						5 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)0 // offset of where the position data begins in the buffer
						);
				
	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	
	//tex coords of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, //location of vertex attribute - which vertex attribute you want to configure
						2, //size of vertex attribute
						GL_FLOAT, //type of the data in vertex attribute
						GL_FALSE, // if we want data to be normalized
						5 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)(3 * sizeof(float)) // offset of where the position data begins in the buffer
						);
				
	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	glBindVertexArray(0);
	
	//configure floor vao
	glGenVertexArrays(1, &gVaoFloor);
	glBindVertexArray(gVaoFloor);
	
	//floor vbo
	glGenBuffers(1, &gVboFloor);
	glBindBuffer(GL_ARRAY_BUFFER, gVboFloor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	loadTexture(&container_texture,"container.jpg");
	loadTexture(&gFloorTexture, "awesomeface.png");
	
	glShadeModel(GL_SMOOTH);
	
	//set-up depth buffer
	glClearDepth(1.0f);
	
	
	
	
	//set really ice perspective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//we will always cull back faces for better performance
	//glEnable(GL_CULL_FACE);
	
	//wireframe model
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	
	//set orthographicMatrix to identify matrix
	gPerspectiveProjectionMatrix = mat4::identity();
	
	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void loadTexture(GLuint *texture, char* path)
{
	//generate texture
	glGenTextures(1, //how many textures 
				texture //texture stored in this
				);
	glBindTexture(GL_TEXTURE_2D, *texture);
	
	//set the texture wrapping/filtering options on the currently bound texture object
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//load image data
	int width, height, nrChannels;
	GLubyte* data = stbi_load(path, //location of the input file
							&width,
							&height,
							&nrChannels, //number of colored channels
							0
							);
	if(data){				
	//start generating texture using previously loaded image data
	glTexImage2D(GL_TEXTURE_2D, //texture target: this operation will generate texture on currently bound texture object at the same target - it won't affect GL_TEXTURE_1D or GL_TEXTURE_3D
				0, //mipmap level for which we want to create a texture for
				GL_RGB, //in what kind of format we want to store the texture
				width, //width of resulting texture
				height, //height of resulting texture
				0, //should always be 0 - some legacy stuff
				GL_RGB, //format of source image
				GL_UNSIGNED_BYTE, //datatype of source image
				data //actual image data
				);
				
	//currently bound texture object now has the base-leve of texture image attached to it
	
	//generate all the required mipmaps for the currently bound texture
	glGenerateMipmap(GL_TEXTURE_2D);
	}else{
		printf("Failed to load the texture");
	}
	//free image memory
	stbi_image_free(data);
}

void display(){
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	
	//depth test to do
	//GL_LESS - depth test passes if fragments z value is less than the depth buffers value - same behaviour we get when we don't enable depth testing
	glDepthFunc(GL_LESS);
	
	//to make read-only depth buffer
	//glDepthMask(GL_FALSE);
	
	//enable stencil testing
	glEnable(GL_STENCIL_TEST);
	
	//
	glStencilOp(GL_KEEP, //sfail - action to take if test fails
				GL_KEEP, //dpfail - action to take if stencil test passes but depth test fails
				GL_REPLACE //dppass - action to take if both the test passes - replace the stored stencil value with the reference value set via glStencilFunc which we later set to 1
				);
				
	glClear(GL_COLOR_BUFFER_BIT //clear color buffer
				| GL_DEPTH_BUFFER_BIT //clear depth buffer as we are using depth testing
			    | GL_STENCIL_BUFFER_BIT //clear stencil buffer as we are using stencil testing
			); 
	
	//make sure we don't update the stencil buffer while drawing the floor
	glStencilMask(0x00);
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);
	
	
	
				
	
				
	
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix=mat4::identity();
	
	
	
	//bind floor vao
	glBindVertexArray(gVaoFloor);
	
	//translate floor
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.0f,0.0f,-3.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//bind texture
	glBindTexture(GL_TEXTURE_2D, gFloorTexture);
	
	//draw floor
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//unbind vao
	glBindVertexArray(0);
	
	
	
	//draw two containers
	//whenever the stencil value of a fragment is equal to reference value 1, the fragment passes the test and is drawn
	//all fragments should update the stencil buffer
	glStencilFunc(GL_ALWAYS, //stencil test function - GL_ALWAYS make sure that each of the containers fragments update the stencil buffer with the stencil value of 1. Tese always passes whenever it is drawn
				1, //stencil buffers content is compared to this value
				0xFF //mask that is ANDed with both the reference values and the stored stencil value before the test compares them
				);
				
	//stencil mask
	//each bit is written to the stencil buffer as is
	//enable writing to the stencil buffer
	glStencilMask(0xFF);
	
	//each bit ends up as 0 in the stencil buffer (disabling writes)
	//glStencilMask(0x00);
	
	//bind vao
	glBindVertexArray(gVao);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	
	
	
	//translate
	modelMatrix = translate(-1.0f,0.0f,-4.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, container_texture);
	glUniform1i(gSamplerUniform, 0);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
				
	//draw second box
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.9f,0.0f,-3.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
				
	//unbind vao
	glBindVertexArray(0);
	
	
	
	//stop using OpenGL program object
	glUseProgram(0);
	
	
	//whenever the stencil value of a fragment is equal to reference value 1, the fragment passes the test and is drawn
	//all fragments should update the stencil buffer
	glStencilFunc(GL_NOTEQUAL, //stencil test function - GL_ALWAYS make sure that we are only drawing the container that are not equal to 1 thus drawing the part of the container that are outside the previously drawn containers.
				1, //stencil buffers content is compared to this value
				0xFF //mask that is ANDed with both the reference values and the stored stencil value before the test compares them
				);
				
	//stencil mask
	//each bit is written to the stencil buffer as is
	//enable writing to the stencil buffer
	glStencilMask(0x00);
	
	glDisable(GL_DEPTH_TEST);
	
	//draw outline
	glUseProgram(gSingleColorShaderProgramObject);
	
	//bind vao
	glBindVertexArray(gVao);
	
	//draw two containers
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	//translate
	modelMatrix = translate(-1.0f,0.0f,-4.0f);
	
	mat4 scaleMatrix = mat4::identity();
	float scale=1.1f;
	scaleMatrix = vmath::scale(scale,scale,scale);
	
	modelMatrix = modelMatrix * scaleMatrix;
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, container_texture);
	glUniform1i(gSamplerUniform, 0);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
				
	//draw second box
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.9f,0.0f,-3.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
				
	//unbind vao
	glBindVertexArray(0);
	
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
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
	
	//destroy vao
	if(gVaoFloor)
	{
		glDeleteVertexArrays(1, &gVaoFloor);
		gVaoFloor = 0;
	}
	
	//destroy vbo
	if(gVboFloor)
	{
		glDeleteBuffers(1, &gVboFloor);
		gVboFloor = 0;
	}
	
	if(gEbo)
	{
		glDeleteBuffers(1, &gEbo);
		gEbo = 0;
	}
	
	if (container_texture)
	{
		glDeleteTextures(1, &container_texture);
		container_texture = 0;
	}
	
	if (gFloorTexture)
	{
		glDeleteTextures(1, &gFloorTexture);
		gFloorTexture = 0;
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