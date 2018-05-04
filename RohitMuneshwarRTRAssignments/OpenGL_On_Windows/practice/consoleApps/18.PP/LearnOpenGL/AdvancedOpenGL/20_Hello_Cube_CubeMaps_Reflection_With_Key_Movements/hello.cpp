//Cube Maps
/*
	It is a texture that contains 6 individual 2D textures that each form one side
	of a cube: texture Cube. They can be indexed/sampled using a direction vector.
	Direction vector to sample the cubemap would be similar to the (interpolated)
	vertex position of the cube. This way we can sample the cubemap using the cubes 
	actual position vectors as long as the cube is centered on the origin.
*/
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

//quad shader
GLuint gQuadVertexShaderObject;
GLuint gQuadFragmentShaderObject;
GLuint gQuadShaderProgramObject;

//skybox shaders
GLuint gVertexShaderObjectSkybox;
GLuint gFragmentShaderObjectSkybox;
GLuint gShaderProgramObjectSkybox;

//box
GLuint gVao;
GLuint gVbo;

//sky box
GLuint gVaoSkybox;
GLuint gVboSkybox;

//floor
GLuint gVaoFloor;
GLuint gVboFloor;

//grass
GLuint gVaoGrass;
GLuint gVboGrass;

//quad
GLuint gVaoQuad;
GLuint gVboQuad;

//frame buffer object
GLuint gFbo;

//color attachment texture
GLuint textureColorBuffer;

//renderbuffer object for depth and stencil attachment - we won't be sampling these
GLuint gRbo;

//set modelview & modelviewprojection matrices to identity
mat4 modelMatrix = mat4::identity();
mat4 rotationMatrix = mat4::identity();
mat4 scaleMatrix = mat4::identity();
mat4 viewMatrix = mat4::identity();
mat4 modelViewProjectionMatrix=mat4::identity();

//uniform variables
GLuint gModelUniform, gViewUniform, gProjectionUniform, gUniformCameraPosition;
GLuint gSamplerUniform, gUniformCubemapSampler, gUniformBoxSkyboxSampler;

//textures
GLuint container_texture, gFloorTexture, gTextureGrass, gTextureCubemap;

//perspective matrix
mat4 gPerspectiveProjectionMatrix;

//box locations
GLuint boxCount = 2;
const vec3 boxLocations[]
{
	//vec3(0.5f,0.0f,-3.6f),
	vec3(0.0f,0.0f,-3.7f),
};

//grass locations
GLuint grassCount = 5;
const vec3 grassPositions[]=
	{
		vec3(-1.0, 0.0f, -3.9f),
		vec3(0.9f, 0.0f, -2.9f),
		vec3(0.0f, 0.0f, -5.7f),
		vec3(0.5f, 0.0f, -4.3f),
		vec3(-3.0f, 0.0f, -6.6f)
	};

//cubemap faces
const char* cubemapFaces[]=
{
	"skybox/right.jpg",
	"skybox/left.jpg",
	"skybox/top.jpg",
	"skybox/bottom.jpg",
	"skybox/front.jpg",
	"skybox/back.jpg"
};
	
//camera movements
//create view matrix
GLfloat rotationAngle=0.0f;
GLfloat radius = 10.0f;
GLfloat cameraSpeed = 0.08f; 
GLfloat camX = sin(rotationAngle) * radius;
GLfloat camZ = cos(rotationAngle) * radius;
vec3 cameraPos = vec3(camX, 0.0f, camZ); //position vector
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f); //direction vector
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0); //up vector

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
				
				case 0x57: //W
				cameraPos = cameraPos + (cameraSpeed * cameraFront);
				break;
				
				case 0x41: //A
				cameraPos = cameraPos - (normalize(cross(cameraFront, cameraUp)) *cameraSpeed);
				break;
				
				case 0x53: //S
				cameraPos = cameraPos - (cameraSpeed * cameraFront);
				break;
				
				case 0x44: //D
				cameraPos = cameraPos + (normalize(cross(cameraFront, cameraUp)) *cameraSpeed);
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
	//function prototypes
	void createShaderPrograObject(void);
	void setUniformLocations(void);
	void configureVaos(void);
	void resize(int,int);
	void uninitialize(void);
	void loadTexture(GLuint *, char* path);
	void loadTextureWithAlpha(GLuint *, char* path);
	void loadCubemap(GLuint *, char* path);
	void configureGLData(void);
	
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
	
	//creat shader progra object
	createShaderPrograObject();
	
	//set uniform locations
	setUniformLocations();
	
	//configure frame buffer object
	glGenFramebuffers(1, &gFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
	
	//create a color attachment texture
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//create a renderbuffer object for depth and stencil attachment - we won't be sampling these
	glGenRenderbuffers(1, &gRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gRbo);
	
	//use single render buffer object for both depth and stencil buffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIN_WIDTH, WIN_HEIGHT);
	
	//now actually attach it
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRbo);
	
	//now that we can actually create the framebuffer and added all attachments we want to check if it is actually competed now
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(g_fp_logfile,"ERROR: FRAMEBUFFER :: Framebuffer is not completed!\n");
	}
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//configure vaos
	configureVaos();
	
	//load texture data
	loadTexture(&container_texture,"container.jpg");
	loadTexture(&gFloorTexture, "ground.jpg");
	loadTextureWithAlpha(&gTextureGrass, "grass.png");
	loadCubemap(&gTextureCubemap, "left.jpg");
	//initialize opengl properties
	configureGLData();
	
	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void configureGLData()
{
	glShadeModel(GL_SMOOTH);
	
	//set-up depth buffer
	glClearDepth(1.0f);
	
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	
	//depth test to do
	//GL_LESS - depth test passes if fragments z value is less than the depth buffers value - same behaviour we get when we don't enable depth testing
	glDepthFunc(GL_LESS);
	
	//to make read-only depth buffer
	//glDepthMask(GL_FALSE);
	
	//set really ice perspective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//we will always cull back faces for better performance
	//glEnable(GL_CULL_FACE);
	
	//wireframe model
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	
	//set orthographicMatrix to identify matrix
	gPerspectiveProjectionMatrix = mat4::identity();
	
	//camera position
	cameraPos = vec3(0.0, 0.2f, -2.0);
}

void display(){
	//function prototypes declaration
	void drawBoxes(void);
	void drawFloor(void);
	void drawGrass(void);
	void drawQuad(void);
	void drawCubemap(void);
	
	//variable initalization
	//set modelview & modelviewprojection matrices to identity
	modelMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix=mat4::identity();

	glClear(GL_COLOR_BUFFER_BIT //clear color buffer
		| GL_DEPTH_BUFFER_BIT //clear depth buffer as we are using depth testing
	); // | GL_STENCIL_BUFFER add kelyawar output disat nhi.... why?
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);

	//OpenGL drawing
	//rotationMatrix = rotate(rotationAngle, 0.5f, 1.0f, 0.0f);

	//modelMatrix = modelMatrix * rotationMatrix;

	//view transformation
	viewMatrix = lookat(cameraPos, //position of camera
		cameraPos + cameraFront, //target - where camera is looking at
		cameraUp //what is up - y axis
	);

	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	glUniformMatrix4fv(gUniformCameraPosition, 1, GL_FALSE, cameraPos);
	//bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTextureCubemap);
	glUniform1i(gUniformBoxSkyboxSampler, 0);
	drawBoxes();//draw boxes
/*
	glBindTexture(GL_TEXTURE_2D, gFloorTexture);//bind texture
	drawFloor();//draw floor

	glBindTexture(GL_TEXTURE_2D, gTextureGrass);//bind texture
	drawGrass();//draw grass
	*/
				//stop using OpenGL program object
	glUseProgram(0);


	glDepthFunc(GL_LEQUAL);

	//draw sky box
	glUseProgram(gShaderProgramObjectSkybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTextureCubemap);
	glUniform1i(gUniformCubemapSampler, 0);
	//view transformation
	/*viewMatrix = mat4::identity();
	viewMatrix = lookat(cameraPos, //position of camera
		cameraPos + cameraFront, //target - where camera is looking at
		cameraUp //what is up - y axis
	);*/
	//viewMatrix = vmath::mat4(vmath::mat3(viewMatrix));
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gViewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	drawCubemap();
	glUseProgram(0);

	glDepthFunc(GL_LESS);
	//swap buffers
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

void createShaderPrograObject()
{
	//function prototypes
	void createVertexShader(void);
	void createFragmentShader(void);
	void createShaderProgram(void);
	
	//code
	createVertexShader();
	createFragmentShader();
	createShaderProgram();
}

void createVertexShader()
{
	//function prototypes
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
		"in vec3 vPosition;	"\
		"in vec3 vNormal;"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"out vec3 out_normals;"\
		"out vec3 out_positions;"\
		"void main(void)						"\
		"{										"\
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"out_normals=mat3(transpose(inverse(u_model_matrix))) * vNormal;"\
		"out_positions=vec3(u_model_matrix * vec4(vPosition, 1.0));"\
				"}										";
				
				
	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode,NULL);
	fprintf(g_fp_logfile,"Actual shadig language version is %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObject);
	
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

	//quad vertex shader
	//create shader
	gQuadVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//provide source code to shader
	const GLchar *vertexShaderSourceCodeQuad =
		"#version 440							"\
		"\n										"\
		"in vec3 vPosition;	"\
		"in vec2 vTexCoords;"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"out vec2 out_tex_coords;"\
		"void main(void)						"\
		"{										"\
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"out_tex_coords=vTexCoords;"\
		"}										";


	glShaderSource(gQuadVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCodeQuad, NULL);
	fprintf(g_fp_logfile, "Actual shadig language version is %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gQuadVertexShaderObject);

	glGetShaderiv(gQuadVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gQuadVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gQuadVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	

	//skybox vertex shader
	//VERTEX SHADER
	//create shader
	gVertexShaderObjectSkybox = glCreateShader(GL_VERTEX_SHADER);

	//provide source code to shader
	const GLchar *vertexShaderSourceCodeSkybox =
		"#version 440							"\
		"\n										"\
		"in vec3 vPosition;	"\
		"uniform mat4 u_model_matrix;"\
		"uniform mat4 u_view_matrix;"\
		"uniform mat4 u_projection_matrix;"\
		"out vec3 out_tex_coords;"\
		"void main(void)						"\
		"{										"\
		"vec4 pos = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition, 1.0);	"\
		"out_tex_coords=vPosition;"\
		"gl_Position = pos.xyww;"\
		"}										";


	glShaderSource(gVertexShaderObjectSkybox, 1, (const GLchar**)&vertexShaderSourceCodeSkybox, NULL);
	fprintf(g_fp_logfile, "Actual shadig language version is %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//compile shader
	glCompileShader(gVertexShaderObjectSkybox);

	glGetShaderiv(gVertexShaderObjectSkybox, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectSkybox, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectSkybox, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "vertex shader compilation log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
}

void createFragmentShader()
{
	//function prototypes
	void uninitialize(void);
	
	//local variable initialization
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
		"in vec3 out_positions;"\
		"in vec3 out_normals;"\
		"uniform samplerCube u_box_skybox_sampler_texture;"\
		"uniform vec3 cameraPos;"\
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"vec3 I = normalize(out_positions - cameraPos);"\
		"vec3 R = reflect(I, normalize(out_normals));"\
		"vec4 texColor = vec4(texture(u_box_skybox_sampler_texture, R).rgb, 1.0);"
		"FragColor=texColor;" \
		"}";
				
	glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode,NULL);
	
	//compile shader
	glCompileShader(gFragmentShaderObject);
	 
	
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



	//QUAD FRAGMENT SHADER
	//create shader
	gQuadFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//provide source code to shader
	const GLchar *fragmentShaderSourceCodeForQuad =
		"#version 440" \
		"\n" \
		"in vec2 out_tex_coords;"\
		"uniform sampler2D u_sampler_texture;"\
		"out vec4 FragColor;" \
		"const float offset = 1.0 / 300.0;"\
		"void main(void)" \
		"{" \
		"vec2 offsets[9] = vec2[]("\
		"vec2(-offset, offset), "\
		"vec2(0.0, offset), "\
		"vec2(offset, offset), "\
		"vec2(-offset, 0.0), "\
		"vec2(0.0, 0.0), "\
		"vec2(offset, 0.0), "\
		"vec2(-offset, -offset), "\
		"vec2(0.0, -offset), "\
		"vec2(offset, -offset) "\
		");"\
		"float kernel[9] = float[]("\
		"1.0, 1.0, 1.0,"\
		"1.0, -8.0, 1.0,"\
		"1.0, 1.0, 1.0"\
		");"\
		"vec3 sampleTex[9];"\
		"for(int i=0; i<9; i++)"\
		"{"\
		"sampleTex[i] = vec3(texture(u_sampler_texture, out_tex_coords + offsets[i]));"\
		"}"\
		"vec3 texColor = vec3(0.0);"
		"for(int i=0; i<9; i++)"\
		"{"\
		"texColor += sampleTex[i] * kernel[i];"\
		"}"\
		"FragColor=vec4(texColor, 1.0);" \
		"}";

	glShaderSource(gQuadFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCodeForQuad, NULL);

	//compile shader
	glCompileShader(gQuadFragmentShaderObject);
	//reinitialize 
	iInfoLogLength = 0;
	iShaderCompileStatus = 0;
	szInfoLog = NULL;
	glGetShaderiv(gQuadFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gQuadFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gQuadFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//Skybox Fragment shader
	//create shader
	gFragmentShaderObjectSkybox = glCreateShader(GL_FRAGMENT_SHADER);

	//provide source code to shader
	const GLchar *fragmentShaderSourceCodeSkybox =
		"#version 440" \
		"\n" \
		"in vec3 out_tex_coords;"\
		"uniform samplerCube u_sampler_cube_texture;"\
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"vec4 texColor = texture(u_sampler_cube_texture, out_tex_coords);"
		"FragColor=texColor;" \
		"}";

	glShaderSource(gFragmentShaderObjectSkybox, 1, (const GLchar**)&fragmentShaderSourceCodeSkybox, NULL);

	//compile shader
	glCompileShader(gFragmentShaderObjectSkybox);


	glGetShaderiv(gFragmentShaderObjectSkybox, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectSkybox, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectSkybox, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
}

void createShaderProgram()
{
	//function prototypes
	void uninitialize(void);
	
	//local variables
	GLint iShaderProgramLinkStatus=0;
	GLint iInfoLogLength;
	char *szInfoLog = NULL;
	
	//code
	//SHADER PROGRAM
	//create
	gShaderProgramObject = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObject,gVertexShaderObject);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObject,gFragmentShaderObject);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0,"vNormal");
	
	//link shader
	glLinkProgram(gShaderProgramObject);
	
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
	
	//shader program object for quad
	
	//create
	gQuadShaderProgramObject = glCreateProgram();
	
	//attach vertex shader to shader program
	glAttachShader(gQuadShaderProgramObject,gQuadVertexShaderObject);

	//attach fragment shader to shader program
	glAttachShader(gQuadShaderProgramObject,gQuadFragmentShaderObject);
	
	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gQuadShaderProgramObject, VDG_ATTRIBUTE_VERTEX,"vPosition");
	glBindAttribLocation(gQuadShaderProgramObject, VDG_ATTRIBUTE_TEXTURE0,"vTexCoords");
	
	//link shader
	glLinkProgram(gQuadShaderProgramObject);
	
	glGetProgramiv(gQuadShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gQuadShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char *)malloc(iInfoLogLength);
			if(szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gQuadShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	//shader program object for sky box
	//SHADER PROGRAM
	//create
	gShaderProgramObjectSkybox = glCreateProgram();

	//attach vertex shader to shader program
	glAttachShader(gShaderProgramObjectSkybox, gVertexShaderObjectSkybox);

	//attach fragment shader to shader program
	glAttachShader(gShaderProgramObjectSkybox, gFragmentShaderObjectSkybox);

	//pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObjectSkybox, VDG_ATTRIBUTE_VERTEX, "vPosition");

	//link shader
	glLinkProgram(gShaderProgramObjectSkybox);

	glGetProgramiv(gShaderProgramObjectSkybox, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectSkybox, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectSkybox, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fp_logfile, "Shader Program Link Log: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
}

void setUniformLocations()
{
	//code
	//get MVP uniform location
	gModelUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gSamplerUniform = glGetUniformLocation(gShaderProgramObject, "u_sampler_texture");
	gUniformCameraPosition = glGetUniformLocation(gShaderProgramObject, "cameraPos");
	gUniformBoxSkyboxSampler = glGetUniformLocation(gShaderProgramObject, "u_box_skybox_sampler_texture");
	gUniformCubemapSampler = glGetUniformLocation(gShaderProgramObjectSkybox, "u_sampler_cube_texture");
}

void configureVaos()
{
	//vertices, colors, shader attribs, vbo, vao initializations
	//for element buffer - see the data is created in anticlock-wise direction
	const GLfloat vertexData[]=
	{
		// positions          // normals
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
	
	//grass vertices
	float grassVertices[]=
	{
		//positions			//texture coordinates
		/*0.0f, 0.5f, 0.0f,   0.0f, 0.0f,
		0.0f, -0.5f, 0.0f,  0.0f, 1.0f,
		1.0f, -0.5f, 0.0f,  1.0f,1.0f,
		
		0.0f, 0.5f, 0.0f,   0.0f, 0.0f,
		0.0f, -0.5f, 0.0f,  1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,   1.0f,0.0f*/
		
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		
		
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		
		
	};
	
	float quadVertices[]=
	{
		//position	//texture coordinates
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
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
						6 * sizeof(float), // stride - the space between consecutive vertex attribute sets
						(void *)0 // offset of where the position data begins in the buffer
						);
				
	//enable vertex attribute by giving vertex attribute location
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	
	//tex coords of vertex
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
	
	//grass vao configuration
	glGenVertexArrays(1, &gVaoGrass);
	glBindVertexArray(gVaoGrass);
	
	//grass vbo
	glGenBuffers(1, &gVboGrass);
	glBindBuffer(GL_ARRAY_BUFFER, gVboGrass);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//grass vao configuration ends
	
	
	//quad vao configuration
	glGenVertexArrays(1, &gVaoQuad);
	glBindVertexArray(gVaoQuad);
	
	//grass vbo
	glGenBuffers(1, &gVboQuad);
	glBindBuffer(GL_ARRAY_BUFFER, gVboQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//quad vao configuration ends


	//skybox vao configuration
	glGenVertexArrays(1, &gVaoSkybox);
	glBindVertexArray(gVaoSkybox);

	//skybox vbo
	glGenBuffers(1, &gVboSkybox);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSkybox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	//glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	//glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//skybox vao configuration ends
	
}
void uninitialize(){
	if(gbFullScreen == true){
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd,GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
	
	//destroy fbo
	if(gFbo)
	{
		glDeleteFramebuffers(1, &gFbo);
		gFbo = 0;
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
	if(gVaoQuad)
	{
		glDeleteVertexArrays(1, &gVaoQuad);
		gVaoQuad = 0;
	}
	
	//destroy vbo
	if(gVboQuad)
	{
		glDeleteBuffers(1, &gVboQuad);
		gVboQuad = 0;
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
	
	//destroy vao
	if(gVaoGrass)
	{
		glDeleteVertexArrays(1, &gVaoGrass);
		gVaoGrass = 0;
	}
	
	//destroy vbo
	if(gVboGrass)
	{
		glDeleteBuffers(1, &gVboGrass);
		gVboGrass = 0;
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
	
	if (gTextureGrass)
	{
		glDeleteTextures(1, &gTextureGrass);
		gTextureGrass = 0;
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

void updateAngle()
{
	rotationAngle = rotationAngle+0.001f;
	if(rotationAngle>=360.0f)
	{
		rotationAngle=rotationAngle-360.0f;
	}
}

void drawBoxes()
{
	//bind vao
	glBindVertexArray(gVao);
	
	for(unsigned int i=0;i<boxCount;i++){
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(boxLocations[i]);
	GLfloat scaleF = 0.5f;
	scaleMatrix = scale(scaleF);
	rotationMatrix = rotate(45.0f, 0.0f, 1.0f, 0.0f);
	modelMatrix = modelMatrix * scaleMatrix * rotationMatrix;
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	
	//draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, //primitive type we like to draw
				 0, //starting index of the vertex array we'd like to draw
				36 //how many vertices we want to draw - we render only one triangle from our data, which is exactly 3 vertices long
				);
	}
	//unbind vao
	glBindVertexArray(0);
}

void drawFloor()
{
	//bind floor vao
	glBindVertexArray(gVaoFloor);
	
	//translate floor
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.0f,0.0f,-3.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	
	//draw floor
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//unbind vao
	glBindVertexArray(0);
}

void drawGrass()
{
	//draw grass
	//bind grass vao
	glBindVertexArray(gVaoGrass);
	
	for(unsigned int i=0;i<grassCount;i++){
	//translate
	modelMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	modelMatrix = translate(grassPositions[i]);
	rotationMatrix = rotate(180.0f, 0.0f, 0.0f, 1.0f);
	modelMatrix = modelMatrix * rotationMatrix;
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	//unbind vao
	glBindVertexArray(0);
}

void drawQuad()
{
	glBindVertexArray(gVaoQuad);
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.0f,0.0f,0.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void drawCubemap()
{
	glBindVertexArray(gVaoSkybox);
	modelMatrix = mat4::identity();
	modelMatrix = translate(0.0f, 0.0f, -3.0f);
	glUniformMatrix4fv(gModelUniform, 1, GL_FALSE, modelMatrix);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
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
	if (data) {
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
	}
	else {
		printf("Failed to load the texture");
	}
	//free image memory
	stbi_image_free(data);
}

void loadTextureWithAlpha(GLuint *texture, char* path)
{
	//generate texture
	glGenTextures(1, //how many textures 
		texture //texture stored in this
	);
	glBindTexture(GL_TEXTURE_2D, *texture);

	//set the texture wrapping/filtering options on the currently bound texture object
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	if (data) {
		//start generating texture using previously loaded image data
		glTexImage2D(GL_TEXTURE_2D, //texture target: this operation will generate texture on currently bound texture object at the same target - it won't affect GL_TEXTURE_1D or GL_TEXTURE_3D
			0, //mipmap level for which we want to create a texture for
			GL_RGBA, //in what kind of format we want to store the texture
			width, //width of resulting texture
			height, //height of resulting texture
			0, //should always be 0 - some legacy stuff
			GL_RGBA, //format of source image
			GL_UNSIGNED_BYTE, //datatype of source image
			data //actual image data
		);

		//currently bound texture object now has the base-leve of texture image attached to it

		//generate all the required mipmaps for the currently bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		printf("Failed to load the texture");
	}
	//free image memory
	stbi_image_free(data);
}

void loadCubemap(GLuint *texture, char* path)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);

	int width, height, nrChannels;
	/*GLubyte *data = stbi_load("skybox/right.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	data = stbi_load("skybox/left.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	data = stbi_load("skybox/top.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	data = stbi_load("skybox/bottom.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	data = stbi_load("skybox/front.jgp", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	data = stbi_load("skybox/back.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);*/
	for (GLuint i=0;i<6;i++)
	{
		GLubyte *data = stbi_load(cubemapFaces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			fprintf(g_fp_logfile, "Cubemap texture failed to load at path: %s", cubemapFaces[i]);
			stbi_image_free(data);
		}
		
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}