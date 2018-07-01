#include<windows.h>
#include<stdio.h> //for file IO
#include<gl/glew.h> //for GLSL extensions
#include<gl/GL.h>

#include "vmath.h"
#include "Sphere.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib,"Sphere.lib")

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
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;

GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;

GLuint gLdUniform;
GLuint gLaUniform;
GLuint gLsUniform;
GLuint gLpUniform;
 
GLuint gKdUniform;
GLuint gKaUniform;
GLuint gKsUniform;
GLuint gKShininessUniform;

GLuint gLKeyPressedUniform;


mat4 gPerspectiveProjectionMatrix;

bool gbLight;

float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_textures[764];
    unsigned short sphere_elements[2280];
	
GLuint gNumElements;
GLuint gNumVertices;

GLfloat light_ambient[]={0.5f,0.5f,0.5f,1.0f};
GLfloat light_defused[]={1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[]={0.0f,0.0f,1.0f,0.0f};

GLfloat s1_material_ambient[]={0.0215f,0.1745f,0.0215f,1.0f};
GLfloat s1_material_diffuse[]={0.07568f,0.61424f,0.07568f,1.0f};
GLfloat s1_material_specular[]={0.633f,0.727811f,0.633f,1.0f};
GLfloat s1_material_shininess=0.6f * 128;

GLfloat s2_material_ambient[]={0.135f,0.2225f,0.1575f,1.0f};
GLfloat s2_material_diffuse[]={0.54f,0.89f,0.63f,1.0f};
GLfloat s2_material_specular[]={0.316228f,0.316228f,0.316228f,1.0f};
GLfloat s2_material_shininess=0.1f * 128;

GLfloat s3_material_ambient[]={0.05375f,0.05f,0.06625f,1.0f};
GLfloat s3_material_diffuse[]={0.18275f,0.17f,0.22525f,1.0f};
GLfloat s3_material_specular[]={0.332741f,0.328634f,0.346435f,1.0f};
GLfloat s3_material_shininess=0.3f * 128;

GLfloat s4_material_ambient[]={0.25f,0.20725f,0.20725f,1.0f};
GLfloat s4_material_diffuse[]={1.0f,0.829f,0.829f,1.0f};
GLfloat s4_material_specular[]={0.296648f,0.296648f,0.296648f,1.0f};
GLfloat s4_material_shininess=0.088f * 128;

GLfloat s5_material_ambient[]={0.1745f,0.01175f,0.01175f,1.0f};
GLfloat s5_material_diffuse[]={0.61424f,0.04136f,0.04136f,1.0f};
GLfloat s5_material_specular[]={0.727811f,0.626959f,0.626959f,1.0f};
GLfloat s5_material_shininess=0.6f * 128;

GLfloat s6_material_ambient[]={0.1f,0.18725f,0.1745f,1.0f};
GLfloat s6_material_diffuse[]={0.396f,0.74151f,0.69102f,1.0f};
GLfloat s6_material_specular[]={0.297254f,0.30829f,0.306678f,1.0f};
GLfloat s6_material_shininess=0.1f * 128;

GLfloat s7_material_ambient[]={0.329412f,0.223529f,0.027451f,1.0f};
GLfloat s7_material_diffuse[]={0.780392f,0.568627f,0.113725f,1.0f};
GLfloat s7_material_specular[]={0.992157f,0.941176f,0.807843f,1.0f};
GLfloat s7_material_shininess=0.21794872f * 128;

GLfloat s8_material_ambient[]={0.2125f,0.1275f,0.054f,1.0f};
GLfloat s8_material_diffuse[]={0.714f,0.4284f,0.18144f,1.0f};
GLfloat s8_material_specular[]={0.393548f,0.271906f,0.166721f,1.0f};
GLfloat s8_material_shininess=0.2f * 128;

GLfloat s9_material_ambient[]={0.25f,0.25f,0.25f,1.0f};
GLfloat s9_material_diffuse[]={0.4f,0.4f,0.4f,1.0f};
GLfloat s9_material_specular[]={0.774597f,0.774597f,0.774597f,1.0f};
GLfloat s9_material_shininess=0.6f * 128;

GLfloat s10_material_ambient[]={0.19125f,0.0735f,0.0225f,1.0f};
GLfloat s10_material_diffuse[]={0.7038f,0.27048f,0.0828f,1.0f};
GLfloat s10_material_specular[]={0.256777f,0.137622f,0.086014f,1.0f};
GLfloat s10_material_shininess=0.1f * 128;

GLfloat s11_material_ambient[]={0.24725f,0.1995f,0.0745f,1.0f};
GLfloat s11_material_diffuse[]={0.75164f,0.60648f,0.22648f,1.0f};
GLfloat s11_material_specular[]={0.628281f,0.555802f,0.366065f,1.0f};
GLfloat s11_material_shininess=0.4f * 128;

GLfloat s12_material_ambient[]={0.19225f,0.19225f,0.19225f,1.0f};
GLfloat s12_material_diffuse[]={0.50754f,0.50754f,0.50754f,1.0f};
GLfloat s12_material_specular[]={0.508273f,0.508273f,0.508273f,1.0f};
GLfloat s12_material_shininess=0.4f * 128;

GLfloat s13_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s13_material_diffuse[]={0.01f,0.01f,0.01f,1.0f};
GLfloat s13_material_specular[]={0.50f,0.50f,0.50f,1.0f};
GLfloat s13_material_shininess=0.25f * 128;

GLfloat s14_material_ambient[]={0.0f,0.1f,0.06f,1.0f};
GLfloat s14_material_diffuse[]={0.0f,0.50980392f,0.50980392f,1.0f};
GLfloat s14_material_specular[]={0.50196078f,0.50196078f,0.50196078f,1.0f};
GLfloat s14_material_shininess=0.25f * 128;

GLfloat s15_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s15_material_diffuse[]={0.1f,0.35f,0.1f,1.0f};
GLfloat s15_material_specular[]={0.45f,0.55f,0.45f,1.0f};
GLfloat s15_material_shininess=0.25f * 128;

GLfloat s16_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s16_material_diffuse[]={0.5f,0.0f,0.0f,1.0f};
GLfloat s16_material_specular[]={0.7f,0.6f,0.6f,1.0f};
GLfloat s16_material_shininess=0.25f * 128;

GLfloat s17_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s17_material_diffuse[]={0.55f,0.55f,0.55f,1.0f};
GLfloat s17_material_specular[]={0.70f,0.70f,0.70f,1.0f};
GLfloat s17_material_shininess=0.25f * 128;

GLfloat s18_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s18_material_diffuse[]={0.5f,0.5f,0.0f,1.0f};
GLfloat s18_material_specular[]={0.60f,0.60f,0.50f,1.0f};
GLfloat s18_material_shininess=0.25f * 128;

GLfloat s19_material_ambient[]={0.02f,0.02f,0.02f,1.0f};
GLfloat s19_material_diffuse[]={0.01f,0.01f,0.01f,1.0f};
GLfloat s19_material_specular[]={0.4f,0.4f,0.4f,1.0f};
GLfloat s19_material_shininess=0.078125f * 128;

GLfloat s20_material_ambient[]={0.0f,0.05f,0.05f,1.0f};
GLfloat s20_material_diffuse[]={0.4f,0.5f,0.5f,1.0f};
GLfloat s20_material_specular[]={0.04f,0.7f,0.7f,1.0f};
GLfloat s20_material_shininess=0.078125f * 128;

GLfloat s21_material_ambient[]={0.0f,0.05f,0.0f,1.0f};
GLfloat s21_material_diffuse[]={0.4f,0.5f,0.4f,1.0f};
GLfloat s21_material_specular[]={0.04f,0.7f,0.04f,1.0f};
GLfloat s21_material_shininess=0.078125f * 128;

GLfloat s22_material_ambient[]={0.05f,0.0f,0.0f,1.0f};
GLfloat s22_material_diffuse[]={0.5f,0.4f,0.4f,1.0f};
GLfloat s22_material_specular[]={0.7f,0.04f,0.04f,1.0f};
GLfloat s22_material_shininess=0.078125f * 128;

GLfloat s23_material_ambient[]={0.05f,0.05f,0.05f,1.0f};
GLfloat s23_material_diffuse[]={0.5f,0.5f,0.5f,1.0f};
GLfloat s23_material_specular[]={0.7f,0.7f,0.7f,1.0f};
GLfloat s23_material_shininess=0.078125f * 128;

GLfloat s24_material_ambient[]={0.05f,0.05f,0.0f,1.0f};
GLfloat s24_material_diffuse[]={0.5f,0.5f,0.4f,1.0f};
GLfloat s24_material_specular[]={0.7f,0.7f,0.04f,1.0f};
GLfloat s24_material_shininess=0.078125f * 128;

float zAxis = -15.0f;
float gRotateRedAngle = 0.0f;
bool gbXPressed=false;
bool gbYPressed=false;
bool gbZPressed=false;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLIne, int iCmdShow){
	void initialize(void);
	void uninitialize(void);
	void display(void);
	void rotate();
	WNDCLASSEX wndclass;
	TCHAR AppName[] = TEXT("Window Custom");
	TCHAR WinName[] = TEXT("PP Lights - Per Fragment Phong Model");
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
				rotate();
				display(); //for double buffer
			}
		}
	}
	
	uninitialize();
	return ((int) msg.wParam);
}

void rotate() {

        gRotateRedAngle = gRotateRedAngle + 0.1f;
        if (gRotateRedAngle >= 360) {
            gRotateRedAngle = 0.0f;
        }

        float angleRedRadian = (float) (3.14f * gRotateRedAngle / 180.0f);

        if (gbXPressed==true) {
            light_position[0] = 0.0f;
            light_position[1] = 100.0f * ((float) cos(angleRedRadian) / 2.0f - (float) sin(angleRedRadian) / 2.0f);
            light_position[2] = 100.0f * ((float) cos(angleRedRadian) / 2.0f + (float) sin(angleRedRadian) / 2.0f);
            light_position[3] = 1.0f;
        } else if (gbYPressed==true) {
            light_position[0] =100.0f * ((float) cos(angleRedRadian)/2.0f + (float) sin(angleRedRadian)/2.0f);
            light_position[1] =0.0f;
            light_position[2] =100.0f * ((float) cos(angleRedRadian)/2.0f - (float) sin(angleRedRadian)/2.0f);
            light_position[3] =1.0f;
        } else if (gbZPressed==true) {
            light_position[0] =100.0f * ((float) cos(angleRedRadian)/2.0f + (float) sin(angleRedRadian)/2.0f);
            light_position[1] =100.0f * ((float) cos(angleRedRadian)/2.0f - (float) sin(angleRedRadian)/2.0f);
            light_position[2] =0.0f;
            light_position[3] =1.0f;
        }



    }

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam){
	void ToggleFullscreen(void);
	void resize(int, int);
	//void display(void);
	void uninitialize(void);
	
	
	
	//variable declarations
	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;
	
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
				case 0x4C: // for 'L' or 'l'
					if (bIsLKeyPressed == false)
					{
						gbLight = true;
						bIsLKeyPressed = true;
					}
					else
					{
						gbLight = false;
						bIsLKeyPressed = false;
					}
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
				case 0x57:
				case 0x77:
					gbXPressed=false;
					gbYPressed=false;
					gbZPressed=false;
				break;
				case 0x58:
				case 0x78:
					gbXPressed=true;
					gbYPressed=false;
					gbZPressed=false;
				break;
				case 0x59:
				case 0x79:
					gbXPressed=false;
					gbYPressed=true;
					gbZPressed=false;
				break;
				case 0x5A:
				case 0x7A:
					gbXPressed=false;
					gbYPressed=false;
					gbZPressed=true;
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
	void drawSphere(void);
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
				"in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform int u_LKeyPressed;"\
				"uniform vec3 u_Lp;"\
				"out vec3 transformed_normals;"\
				"out vec3 light_direction;"\
				"out vec3 viewer_vector;"\
				"void main(void)						"\
				"{										"\
				"if(u_LKeyPressed == 1)"\
				"{"\
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"\
				"transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;"\
				"light_direction = vec3(u_Lp) - eyeCoordinates.xyz;"\
				"viewer_vector = -eyeCoordinates.xyz;"\
				"}"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"\
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
				"in vec3 transformed_normals;"\
				"in vec3 light_direction;"\
				"in vec3 viewer_vector;"\
				"out vec4 FragColor;" \
				"uniform vec3 u_La;"\
				"uniform vec3 u_Ld;"\
				"uniform vec3 u_Ls;"\
				"uniform vec3 u_Ka;"\
				"uniform vec3 u_Kd;"\
				"uniform vec3 u_Ks;"\
				"uniform float u_KShininess;"\
				"uniform int u_LKeyPressed;"
				"void main(void)" \
				"{" \
				"vec3 phong_ads_color;"\
				"if(u_LKeyPressed==1)"\
				"{"\
				"vec3 normalized_transformed_normals=normalize(transformed_normals);"\
				"vec3 normalized_light_direction=normalize(light_direction);"\
				"vec3 normalized_viewer_vector=normalize(viewer_vector);"\
				"vec3 ambient=u_La * u_Ka;"\
				"float tn_dot_id = max(dot(normalized_transformed_normals,normalized_light_direction),0.0);"\
				"vec3 diffuse = u_Ld * u_Kd * tn_dot_id;"\
				"vec3 reflection_vector=reflect(-normalized_light_direction, normalized_transformed_normals);"\
				"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector),0.0), u_KShininess);"\
				"phong_ads_color=ambient+diffuse+specular;"\
				"}"\
				"else"\
				"{"\
				"phong_ads_color=vec3(1.0,1.0,1.0);"\
				"}"\
				"FragColor=vec4(phong_ads_color, 1.0);" \
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
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	
	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
	
	gLaUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	gLsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	gLpUniform = glGetUniformLocation(gShaderProgramObject, "u_Lp");
	
	gKaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	gKsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	gKShininessUniform = glGetUniformLocation(gShaderProgramObject, "u_KShininess");
	
	drawSphere();
	
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
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	//set orthographicMatrix to identify matrix
	gPerspectiveProjectionMatrix = mat4::identity();
	
	gbLight = false;
	//resize
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void drawSphere(){
	// *** vertices, colors, shader attribs, vbo, vao initializations ***
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();
	// vao
    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);

    // position vbo
    glGenBuffers(1, &gVbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // normal vbo
    glGenBuffers(1, &gVbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // element vbo
    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);
}
void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // | GL_STENCIL_BUFFER add kelyawar output disat nhi.... why?
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);
	
	if(gbLight==true)
	{
		glUniform1i(gLKeyPressedUniform, 1);
		glUniform3fv(gLaUniform, 1, light_ambient);
		glUniform3fv(gLdUniform, 1, light_defused);
		glUniform3fv(gLsUniform, 1, light_specular);
		glUniform3fv(gLpUniform, 1, light_position);
	}else
	{
		glUniform1i(gLKeyPressedUniform,0);
	}
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	
	//1st row
	glUniform3fv(gKaUniform, 1, s1_material_ambient);
	glUniform3fv(gKdUniform, 1, s1_material_diffuse);
	glUniform3fv(gKsUniform, 1, s1_material_specular);
	glUniform1f(gKShininessUniform, s1_material_shininess);
	//translate
	modelMatrix = translate(-3.75f, 3.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s2_material_ambient);
	glUniform3fv(gKdUniform, 1, s2_material_diffuse);
	glUniform3fv(gKsUniform, 1, s2_material_specular);
	glUniform1f(gKShininessUniform, s2_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, 3.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s24_material_ambient);
	glUniform3fv(gKdUniform, 1, s24_material_diffuse);
	glUniform3fv(gKsUniform, 1, s24_material_specular);
	glUniform1f(gKShininessUniform, s2_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, 3.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	glUniform3fv(gKaUniform, 1, s3_material_ambient);
	glUniform3fv(gKdUniform, 1, s3_material_diffuse);
	glUniform3fv(gKsUniform, 1, s3_material_specular);
	glUniform1f(gKShininessUniform, s3_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, 3.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	
	
	
	//2nd row
	//translate
	glUniform3fv(gKaUniform, 1, s4_material_ambient);
	glUniform3fv(gKdUniform, 1, s4_material_diffuse);
	glUniform3fv(gKsUniform, 1, s4_material_specular);
	glUniform1f(gKShininessUniform, s4_material_shininess);
	modelMatrix = translate(-3.75f, 2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	glUniform3fv(gKaUniform, 1, s5_material_ambient);
	glUniform3fv(gKdUniform, 1, s5_material_diffuse);
	glUniform3fv(gKsUniform, 1, s5_material_specular);
	glUniform1f(gKShininessUniform, s5_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, 2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s6_material_ambient);
	glUniform3fv(gKdUniform, 1, s6_material_diffuse);
	glUniform3fv(gKsUniform, 1, s6_material_specular);
	glUniform1f(gKShininessUniform, s6_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, 2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	//translate
	glUniform3fv(gKaUniform, 1, s7_material_ambient);
	glUniform3fv(gKdUniform, 1, s7_material_diffuse);
	glUniform3fv(gKsUniform, 1, s7_material_specular);
	glUniform1f(gKShininessUniform, s7_material_shininess);
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, 2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	
	
	
	//3rd row
	glUniform3fv(gKaUniform, 1, s8_material_ambient);
	glUniform3fv(gKdUniform, 1, s8_material_diffuse);
	glUniform3fv(gKsUniform, 1, s8_material_specular);
	glUniform1f(gKShininessUniform, s8_material_shininess);
	//translate
	modelMatrix = translate(-3.75f, 1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	//translate
	glUniform3fv(gKaUniform, 1, s9_material_ambient);
	glUniform3fv(gKdUniform, 1, s9_material_diffuse);
	glUniform3fv(gKsUniform, 1, s9_material_specular);
	glUniform1f(gKShininessUniform, s9_material_shininess);
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, 1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s10_material_ambient);
	glUniform3fv(gKdUniform, 1, s10_material_diffuse);
	glUniform3fv(gKsUniform, 1, s10_material_specular);
	glUniform1f(gKShininessUniform, s10_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, 1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s11_material_ambient);
	glUniform3fv(gKdUniform, 1, s11_material_diffuse);
	glUniform3fv(gKsUniform, 1, s11_material_specular);
	glUniform1f(gKShininessUniform, s11_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, 1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	
	glUniform3fv(gKaUniform, 1, s12_material_ambient);
	glUniform3fv(gKdUniform, 1, s12_material_diffuse);
	glUniform3fv(gKsUniform, 1, s12_material_specular);
	glUniform1f(gKShininessUniform, s12_material_shininess);
	//4th row
	//translate
	modelMatrix = translate(-3.75f, 0.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s13_material_ambient);
	glUniform3fv(gKdUniform, 1, s13_material_diffuse);
	glUniform3fv(gKsUniform, 1, s13_material_specular);
	glUniform1f(gKShininessUniform, s13_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, 0.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s14_material_ambient);
	glUniform3fv(gKdUniform, 1, s14_material_diffuse);
	glUniform3fv(gKsUniform, 1, s14_material_specular);
	glUniform1f(gKShininessUniform, s14_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, 0.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s15_material_ambient);
	glUniform3fv(gKdUniform, 1, s15_material_diffuse);
	glUniform3fv(gKsUniform, 1, s15_material_specular);
	glUniform1f(gKShininessUniform, s15_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, 0.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	
	glUniform3fv(gKaUniform, 1, s16_material_ambient);
	glUniform3fv(gKdUniform, 1, s16_material_diffuse);
	glUniform3fv(gKsUniform, 1, s16_material_specular);
	glUniform1f(gKShininessUniform, s16_material_shininess);
	//5th row
	//translate
	modelMatrix = translate(-3.75f, -1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s17_material_ambient);
	glUniform3fv(gKdUniform, 1, s17_material_diffuse);
	glUniform3fv(gKsUniform, 1, s17_material_specular);
	glUniform1f(gKShininessUniform, s17_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, -1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s18_material_ambient);
	glUniform3fv(gKdUniform, 1, s18_material_diffuse);
	glUniform3fv(gKsUniform, 1, s18_material_specular);
	glUniform1f(gKShininessUniform, s18_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, -1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	glUniform3fv(gKaUniform, 1, s19_material_ambient);
	glUniform3fv(gKdUniform, 1, s19_material_diffuse);
	glUniform3fv(gKsUniform, 1, s19_material_specular);
	glUniform1f(gKShininessUniform, s19_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, -1.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	
	
	glUniform3fv(gKaUniform, 1, s20_material_ambient);
	glUniform3fv(gKdUniform, 1, s20_material_diffuse);
	glUniform3fv(gKsUniform, 1, s20_material_specular);
	glUniform1f(gKShininessUniform, s20_material_shininess);
	//6th row
	//translate
	modelMatrix = translate(-3.75f, -2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s21_material_ambient);
	glUniform3fv(gKdUniform, 1, s21_material_diffuse);
	glUniform3fv(gKsUniform, 1, s21_material_specular);
	glUniform1f(gKShininessUniform, s21_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(-1.25f, -2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s22_material_ambient);
	glUniform3fv(gKdUniform, 1, s22_material_diffuse);
	glUniform3fv(gKsUniform, 1, s22_material_specular);
	glUniform1f(gKShininessUniform, s22_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(1.25f, -2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);
	
	
	glUniform3fv(gKaUniform, 1, s23_material_ambient);
	glUniform3fv(gKdUniform, 1, s23_material_diffuse);
	glUniform3fv(gKsUniform, 1, s23_material_specular);
	glUniform1f(gKShininessUniform, s23_material_shininess);
	//translate
	modelMatrix = mat4::identity();
	modelMatrix = translate(3.75f, -2.0f, zAxis);
	
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	
	// *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
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
	if(gVao_sphere)
	{
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}
	
	//destroy vbo
	if(gVbo_sphere_position)
	{
		glDeleteBuffers(1, &gVbo_sphere_position);
		gVbo_sphere_position = 0;
	}
	
	if(gVbo_sphere_normal)
	{
		glDeleteBuffers(1, &gVbo_sphere_normal);
		gVbo_sphere_normal = 0;
	}
	
	if(gVbo_sphere_element)
	{
		glDeleteBuffers(1, &gVbo_sphere_element);
		gVbo_sphere_element = 0;
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
