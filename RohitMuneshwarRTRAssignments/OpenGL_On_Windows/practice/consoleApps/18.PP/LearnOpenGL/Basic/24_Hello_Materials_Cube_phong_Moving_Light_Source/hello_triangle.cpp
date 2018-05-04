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

GLuint gFragmentShaderObject2;
GLuint gShaderProgramObject2;

GLuint gVao_object;
GLuint gVao_light;
GLuint gVbo;
GLuint gEbo;
GLuint gVbo_col;
GLuint gModelUniform, gViewUniform, gProjectionUniform;
GLuint gObjectColorUniform, gLightColorUniform, gLightPositionUniform, gViewerPositionUniform;
GLuint gLightAmbientUniform, gLightDiffuseUniform, gLightSpecularUniform;
GLuint gMaterialAmbientUniform, gMaterialDiffuseUniform, gMaterialSpecularUniform, gMaterialShininessUniform;
mat4 gPerspectiveProjectionMatrix;
GLfloat rotationAngle=0.0f;
vec3 lightPos = vec3(1.2f, 0.5f, -3.0f);

vec3 material_ambient=vec3(1.0f, 0.5f, 0.31f);
vec3 material_diffuse=vec3(1.0f, 0.5f, 0.31f);
vec3 material_specular=vec3(0.5f, 0.5f, 0.5f);
float material_shininess=32.0f;

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
	rotationAngle = rotationAngle+0.001f;
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
				"in vec3 vPosition;"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"out vec3 out_normal;"\
				"out vec3 out_fragPos;"\
				"void main(void)						"\
				"{										"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
				"out_normal = mat3(transpose(inverse(u_model_matrix))) * vNormal;"\
				"out_fragPos = vec3(u_model_matrix * vec4(vPosition, 1.0));"\
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
				"in vec3 out_normal;"\
				"in vec3 out_fragPos;"\
				"uniform vec3 u_object_color;"\
				"uniform vec3 u_light_ambient;"\
				"uniform vec3 u_light_diffuse;"\
				"uniform vec3 u_light_specular;"\
				"uniform vec3 u_light_position;"\
				"uniform vec3 u_viewer_position;"\
				"out vec4 FragColor;" \
				"uniform vec3 u_material_ambient;"\
				"uniform vec3 u_material_diffuse;"\
				"uniform vec3 u_material_specular;"\
				"uniform float u_material_shininess;"\
				"void main(void)" \
				"{" \
				"vec3 ambient =  u_light_ambient * u_material_ambient;"\
				
				"vec3 norm = normalize(out_normal);"\
				"vec3 lightDir = normalize(u_light_position - out_fragPos);"\
				"float diff = max(dot(norm, lightDir), 0.0);"\
				"vec3 diffuse = u_light_diffuse * (diff * u_material_diffuse);"\
				
				"vec3 viewDir = normalize(u_viewer_position - out_fragPos);"\
				"vec3 reflectDir = reflect(-lightDir, norm);"\
				"float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material_shininess);"\
				"vec3 specular = u_light_specular * (spec * u_material_specular);"\
				
				"vec3 result = ambient + diffuse + specular;"\
				"FragColor=vec4(result, 1.0);" \
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
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL,"vNormal");
	
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
	
	//get MVP uniform location
	gModelUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gObjectColorUniform = glGetUniformLocation(gShaderProgramObject, "u_object_color");
	gLightAmbientUniform = glGetUniformLocation(gShaderProgramObject, "u_light_ambient");
	gLightDiffuseUniform = glGetUniformLocation(gShaderProgramObject, "u_light_diffuse");
	gLightSpecularUniform = glGetUniformLocation(gShaderProgramObject, "u_light_specular");
	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");
	gViewerPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_viewer_position");
	gMaterialAmbientUniform = glGetUniformLocation(gShaderProgramObject, "u_material_ambient");
	gMaterialDiffuseUniform = glGetUniformLocation(gShaderProgramObject, "u_material_diffuse");
	gMaterialSpecularUniform = glGetUniformLocation(gShaderProgramObject, "u_material_specular");
	gMaterialShininessUniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");
	
	
	//creates Second shader program object
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObject2 = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *fragmentShaderSourceCode2 =
				"#version 440" \
				"\n" \
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(1.0);" \
				"}";
				
	glShaderSource(gFragmentShaderObject2, 1, (const GLchar**)&fragmentShaderSourceCode2,NULL);
	
	//compile shader
	glCompileShader(gFragmentShaderObject2);
	//reinitialize 
	 iInfoLogLength = 0;
	 iShaderCompileStatus = 0;
	 szInfoLog = NULL;
	glGetShaderiv(gFragmentShaderObject2, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject2, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject2, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile,"Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//SHADER PROGRAM
	//create
	gShaderProgramObject2 = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObject2,gVertexShaderObject);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObject2,gFragmentShaderObject2);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject2, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gShaderProgramObject2, VDG_ATTRIBUTE_NORMAL,"vNormal");
	
	//link shader
	glLinkProgram(gShaderProgramObject2);
	iShaderProgramLinkStatus=0;
	glGetProgramiv(gShaderProgramObject2, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject2, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject2, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//vertices, colors, shader attribs, vbo, vao initializations
	//for element buffer - see the data is created in anticlock-wise direction
	const GLfloat vertexData[]=
	{
	//position			  //normal
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	
	//vao
	glGenVertexArrays(1, &gVao_object);
	glBindVertexArray(gVao_object);
	
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
						6 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)0 // offset of where the position data begins in the buffer
						);
				
	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	//normal of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, //location of vertex attribute - which vertex attribute you want to configure
						3, //size of vertex attribute
						GL_FLOAT, //type of the data in vertex attribute
						GL_FALSE, // if we want data to be normalized
						6 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)(3 * sizeof(float)) // offset of where the position data begins in the buffer
						);
				
	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
	
	//vao light
	glGenVertexArrays(1, &gVao_light);
	glBindVertexArray(gVao_light);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo);
	
	//tell OpenGL how it should interpret the vertex data (per vertex attribute)
	//position of vertex
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, //location of vertex attribute - which vertex attribute you want to configure
						3, //size of vertex attribute
						GL_FLOAT, //type of the data in vertex attribute
						GL_FALSE, // if we want data to be normalized
						6 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)0 // offset of where the position data begins in the buffer
						);
						
	//enable vertex attribute by giving vertex attribute location
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
	//glEnable(GL_CULL_FACE);
	
	//wireframe model
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(0.2f,0.3f,0.3f,1.0f);
	
	//set orthographicMatrix to identify matrix
	gPerspectiveProjectionMatrix = mat4::identity();
	
	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT //clear color buffer
				| GL_DEPTH_BUFFER_BIT //clear depth buffer as we are using depth testing
			); // | GL_STENCIL_BUFFER add kelyawar output disat nhi.... why?
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix=mat4::identity();
	
	//translate
	modelMatrix = translate(0.0f,0.0f,-3.0f);
	
	//scale light source
	rotationMatrix = rotate(180.0f, 1.0f, 0.0f, 0.3f);
	
	modelMatrix = modelMatrix * rotationMatrix;
	
	//create view matrix
	GLfloat radius = 10.0f;
	GLfloat camX = sin(rotationAngle) * radius;
	GLfloat camZ = cos(rotationAngle) * radius;
	vec3 cameraPos = vec3(0.0f, 0.0f, 0.0f);//vec3(camX, 0.0f, camZ);
	vec3 cameraFront = vec3(0.0f, 0.0f, 0.0f);
	vec3 cameraUp = vec3(0.0f, 1.0f, 0.0);
	/*viewMatrix = lookat( cameraPos, //position of camera
					cameraFront, //target - where camera is looking at
					cameraUp //what is up - y axis
					);*/
	
	//multiply the modelview and perspective matrix to get modelviewprojection matrix
	//modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelMatrix; //order is important
	
	//change the lights position values over time
	lightPos[0] = 1.0f + sin(rotationAngle) * 2.0f;
	lightPos[1] = sin(rotationAngle/2.0f) * 1.0f;
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	glUniform3fv(gObjectColorUniform, 1, vec3(1.0f, 0.5f, 0.31f));
	glUniform3fv(gLightAmbientUniform, 1, vec3(0.2f, 0.2f, 0.2f));
	glUniform3fv(gLightDiffuseUniform, 1, vec3(0.5f, 0.5f, 0.5f));
	glUniform3fv(gLightSpecularUniform, 1, vec3(1.0f, 1.0f, 1.0f));
	glUniform3fv(gLightPositionUniform, 1, lightPos);
	glUniform3fv(gViewerPositionUniform, 1, cameraPos);
	glUniform3fv(gMaterialAmbientUniform, 1, material_ambient);
	glUniform3fv(gMaterialDiffuseUniform, 1, material_diffuse);
	glUniform3fv(gMaterialSpecularUniform, 1, material_specular);
	glUniform1f(gMaterialShininessUniform, material_shininess);
	glBindVertexArray(gVao_object);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
	
	//unbind vao
	glBindVertexArray(0);
	
	//stop using OpenGL program object
	glUseProgram(0);
	
	
	//use second program object to create light source
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject2);
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	modelMatrix = mat4::identity();
	mat4 scaleMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix=mat4::identity();
	
	//translate
	modelMatrix = translate(lightPos);
	
	//scale light source
	scaleMatrix = scale(vec3(0.2f));
	
	modelMatrix = modelMatrix * scaleMatrix;
	//multiply the modelview and perspective matrix to get modelviewprojection matrix
	//modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelMatrix; //order is important
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	glBindVertexArray(gVao_light);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
	
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
	if(gVao_object)
	{
		glDeleteVertexArrays(1, &gVao_object);
		gVao_object = 0;
	}
	
	if(gVao_light)
	{
		glDeleteVertexArrays(1, &gVao_light);
		gVao_light = 0;
	}
	
	//destroy vbo
	if(gVbo)
	{
		glDeleteBuffers(1, &gVbo);
		gVbo = 0;
	}
	
	if(gEbo)
	{
		glDeleteBuffers(1, &gEbo);
		gEbo = 0;
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
	
	
	//detach vertex shader from shader program object
	glDetachShader(gShaderProgramObject2, gVertexShaderObject);
	
	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObject2, gFragmentShaderObject2);
	
	//delete fragment shader object
	glDeleteShader(gFragmentShaderObject2);
	gFragmentShaderObject2=0;
	
	//delete shader program object
	glDeleteProgram(gShaderProgramObject2);
	gShaderProgramObject2=0;
	

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