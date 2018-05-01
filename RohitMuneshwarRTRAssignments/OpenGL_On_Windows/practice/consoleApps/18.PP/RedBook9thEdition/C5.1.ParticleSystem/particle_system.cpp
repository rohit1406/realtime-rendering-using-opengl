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
#define BUFFER_OFFSET(offset) ((void *)(offset))

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX = 0,
	VDG_ATTRIBUTE_COLOR = 1,
	VDG_ATTRIBUTE_NORMAL = 2,
	VDG_ATTRIBUTE_TEXTURE0 = 3
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

GLuint gVertexShaderObjectGeometryPass;
GLuint gFragmentShaderObjectGeometryPass;
GLuint gShaderProgramObjectGeometryPass;
GLuint gVertexShaderObjectSimulationPass;
GLuint gFragmentShaderObjectSimulationPass;
GLuint gShaderProgramObjectSimulationPass;

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
	
	//VERTEX SHADER Geometry Pass
	//create shader
	gVertexShaderObjectGeometryPass = glCreateShader(GL_VERTEX_SHADER);
	
	//provide source code to shader
	/*const GLchar *vertexShaderSourceCode =
				"#version 440							"\
				"\n										"\
				"in vec4 vPosition;						"\
				"in vec4 vColor;"\
				"uniform mat4 u_mvp_matrix;				"\
				"out vec4 fs_color;"\
				"void main(void)						"\
				"{										"\
				"gl_Position = u_mvp_matrix * vPosition;	"\
				"fs_color = vColor;"\
				"}										";*/
				
	const GLchar *vertexShaderForGeometryPassParticleSystemSim =
				"#version 440"\
				"\n"\
				"in vec4 vPosition;"\
				"in vec3 vNormal;"\
				"uniform mat4 model_matrix;"\
				"uniform mat4 projection_matrix;"\
				"out vec4 world_space_position;"\
				"out vec3 vs_fs_normal;"\
				"void main(void)"\
				"{"\
				"vec4 pos = (model_matrix * (vPosition * vec4(1.0,1.0,1.0,1.0)));"\
				"world_space_position=pos;"\
				"vs_fs_normal=normalize((model_matrix * vec4(vNormal, 0.0)).xyz);"\
				"gl_Position=projection_matrix * pos;"\
				"}";
	
	
	
	glShaderSource(gVertexShaderObjectGeometryPass, 1, (const GLchar**)&vertexShaderForGeometryPassParticleSystemSim,NULL);
	fprintf(g_fp_logfile,"Actual shadig language version is %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObjectGeometryPass);
	
	GLint iInfoLogLength=0;
	GLint iShaderCompileStatus=0;
	char *szInfoLog=NULL;
	glGetShaderiv(gVertexShaderObjectGeometryPass, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectGeometryPass, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectGeometryPass, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				
				uninitialize();
				exit(0);
			}
		}
	}
	
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObjectGeometryPass = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *fragmentShaderSourceCode =
				"#version 440" \
				"\n" \
				"in vec4 fs_color;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=fs_color;" \
				"}";
				
	glShaderSource(gFragmentShaderObjectGeometryPass, 1, (const GLchar**)&fragmentShaderSourceCode,NULL);
	
	//compile shader
	glCompileShader(gFragmentShaderObjectGeometryPass);
	//reinitialize 
	 iInfoLogLength = 0;
	 iShaderCompileStatus = 0;
	 szInfoLog = NULL;
	glGetShaderiv(gFragmentShaderObjectGeometryPass, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectGeometryPass, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectGeometryPass, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile,"Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//SHADER PROGRAM
	//create
	gShaderProgramObjectGeometryPass = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObjectGeometryPass,gVertexShaderObjectGeometryPass);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObjectGeometryPass,gFragmentShaderObjectGeometryPass);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObjectGeometryPass, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gShaderProgramObjectGeometryPass, VDG_ATTRIBUTE_VERTEX,"vColor");
	
	//configure geometry pass of the particle system simulator
	static const char* varyings2[] =
	{
		"world_space_position"
	}
	glTransformFeedbackVaryings(gShaderProgramObjectGeometryPass, 1, varyings2, GL_INTERLEAVED_ATTRIBS);
	
	//link shader
	glLinkProgram(gShaderProgramObjectGeometryPass);
	GLint iShaderProgramLinkStatus=0;
	glGetProgramiv(gShaderProgramObjectGeometryPass, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectGeometryPass, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectGeometryPass, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//get MVP uniform location
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	
	
	
	
	//Shaders for Simulation Pass
	const GLchar *vertexShaderForSimulatinPassParticleSystemSim =
				"#version 440"\
				"\n"\
				"in vec4 vPosition;"\
				"in vec3 vVelocity;"\
				"uniform mat4 model_matrix;"\
				"uniform mat4 projection_matrix;"\
				"uniform int triangle_count;"\
				"uniform samplerBuffer geometry_tbo;"\
				"uniform float time_step=0.02;"\
				"out vec4 position_out;"\
				"out vec3 velocity_out;"\
				"bool intersect(vec3 origin, vec3 direction, vec3 v0, vec3 v1, vec3 v2, out vec3 point)"\
				"{"\
				"vec3 u, v, n;"\
				"vec3 w0, w;"\
				"float r, a, b;"\
				"u = (v1 - v0);"\
				"v = (v2 - v0);"\
				"n = cross(u, v);"\
				"w0 = origin - v0;"\
				"a = -dot(n, w0);"\
				"b = dot(n, direction);"\
				"r = a/b;"\
				"if(r<0.0 || r>1.0)"\
				"{"\
				"return false;"\
				"}"\
				"point = origin + r * direction;"\
				"float uu, uv, vv, wu, wv, D;"\
				"uu = dot(u, u);"\
				"uv = dot(u, v);"\
				"vv = dot(v, v);"\
				"w = point - v0;"\
				"wu = dot(w, u);"\
				"wv = dot(w, v);"\
				"D = uv * uv - vv * wu;"\
				"float s,t;"\
				"s=(uv * wv - vv * wu)/D;"\
				"if(s<0.0 || s>1.0)"\
				"{"\
				"return false;"\
				"}"\
				"t=(uv * wu - uu * wv)/D;"\
				"if(t<0.0 || (s+t)>1.0)"\
				"{"\
				"return false;"\
				"}"\
				"return true;"\
				"}"\
				"vec3 reflect_vector(vec3 v, vec3 n)"\
				"{"\
				"return v - 2.0 * dot(v,n) * n;"\
				"}"\
				"void main(void)"\
				"{"\
				"vec3 acceleration = vec3(0.0, -0.3, 0.0);"\
				"vec3 new_velocity = vVelocity + acceleration * time_step;"\
				"vec4 new_position = vPosition + vec4(new_velocity * time_step, 0.0);"\
				"vec3 v0, v1, v2;"\
				"vec3 point;"\
				"int i;"\
				"for(i-0; i<triangle_count; i++)"\
				"{"\
				"v0 = texelFetch(geometry_tbo, i*3).xyz;"\
				"v1 = texelFetch(geometry_tbo, i*3+1).xyz;"\
				"v2 = texelFetch(geometry_tbo, i*3+2).xyz;"\
				"if(intersect(vPosition.xyz, vPosition.xyz - new_position.xyz, v0, v1, v2, point))"\
				"{"
				"vec3 n = normalize(cross(v1-v0, v2-v0));"
				"new_position=vec4(point + reflect_vector(new_position.xyz - point, n), 1.0);"\
				"new_velocity=0.8 * reflect_vector(new_velocity, n);"\
				"}"\
				"}"\
				"if(new_position.y < -40.0)"\
				"{"\
				"new_position = vec4(-new_position.x * 0.3, position.y + 80.0, 0.0, 1.0);"\
				"new_velocity *= vec3(0.2, 0.1,-0.3);"\
				"velocity_out = new_velocity * 0.9999;"\
				"position_out = new_position;"\
				"gl_Position = projection_matrix * (model_matrix * vPosition);"\
				"}"\
				"}";
	
	//create shader
	gVertexShaderObjectSimulationPasss = glCreateShader(GL_VERTEX_SHADER);
	
	glShaderSource(gVertexShaderObjectSimulationPasss, 1, (const GLchar**)&vertexShaderForSimulatinPassParticleSystemSim,NULL);
	fprintf(g_fp_logfile,"Actual shadig language version is %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObjectSimulationPasss);
	
	GLint iInfoLogLength=0;
	GLint iShaderCompileStatus=0;
	char *szInfoLog=NULL;
	glGetShaderiv(gVertexShaderObjectSimulationPasss, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectSimulationPasss, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectSimulationPasss, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				
				uninitialize();
				exit(0);
			}
		}
	}
	
	//FRAGMENT SHADER
	//create shader
	gFragmentShaderObjectSimulationPass = glCreateShader(GL_FRAGMENT_SHADER);
	
	//provide source code to shader
	const GLchar *fragmentShaderSourceCode =
				"#version 440" \
				"\n" \
				"in vec4 fs_color;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=fs_color;" \
				"}";
				
	glShaderSource(gFragmentShaderObjectSimulationPass, 1, (const GLchar**)&fragmentShaderSourceCode,NULL);
	
	//compile shader
	glCompileShader(gFragmentShaderObjectSimulationPass);
	//reinitialize 
	 iInfoLogLength = 0;
	 iShaderCompileStatus = 0;
	 szInfoLog = NULL;
	glGetShaderiv(gFragmentShaderObjectSimulationPass, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus==GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectSimulationPass, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectSimulationPass, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile,"Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	
	//SHADER PROGRAM
	//create
	gShaderProgramObjectSimulationPass = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObjectSimulationPass,gVertexShaderObjectSimulationPasss);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObjectSimulationPass,gFragmentShaderObjectSimulationPass);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObjectSimulationPass, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gShaderProgramObjectSimulationPass, VDG_ATTRIBUTE_VERTEX,"vColor");
	
	//configure the simulation pass of the particle system simulator
	static const char* varyings[]=
	{
		"position_out", "velocity_out"
	}
	glTransformFeedbackVaryings(gShaderProgramObjectSimulationPass, 2, varyings, GL_INTERLEAVED_ATTRIBS);
	
	//link shader
	glLinkProgram(gShaderProgramObjectSimulationPass);
	GLint iShaderProgramLinkStatus=0;
	glGetProgramiv(gShaderProgramObjectSimulationPass, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectSimulationPass, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectSimulationPass, iInfoLogLength, &written, szInfoLog);
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
	struct VertexData
	{
		GLubyte color[4];
		GLfloat position[4];
	};
	
	VertexData vertices[6]=
	{
		{{255,0,0,255},{-0.90,-0.90}},
		{{0,255,0,255},{0.85,-0.90}},
		{{0,0,255,255},{-0.90,0.85}},
		{{10,10,10,255},{0.90,-0.85}},
		{{100,100,100,255},{0.90,0.90}},
		{{255,255,255,255},{-0.85,0.90}}
	};
	
	glGenVertexArrays(1, &gVao);
	glBindVertexArray(gVao);
	
	glGenBuffers(1, &gVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), BUFFER_OFFSET(0));
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(vertices[0].color)));
	glEnableVertexAttribArray(VDG_ATTRIBUTE_COLOR);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glShadeModel(GL_SMOOTH);
	
	//set-up depth buffer
	glClearDepth(1.0f);
	
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	
	//enable scissor test
	//glEnable(GL_SCISSOR_TEST);
	//glScissor(400,300, 400, 300);
	//depth test to do
	glDepthFunc(GL_LEQUAL);
	
	//set really ice perspective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//we will always cull back faces for better performance
	glEnable(GL_CULL_FACE);
	
	glClearColor(0.25f,0.25f,0.25f,0.0f);
	
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
	modelViewMatrix = translate(0.0f,0.0f,-3.0f);
	
	//multiply the modelview and perspective matrix to get modelviewprojection matrix
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //order is important
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	
	//bind vao
	glBindVertexArray(gVao);
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, 0,6);
	
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