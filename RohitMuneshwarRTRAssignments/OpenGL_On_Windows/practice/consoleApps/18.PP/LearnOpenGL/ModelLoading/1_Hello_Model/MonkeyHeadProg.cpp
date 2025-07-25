#include<Windows.h>

#include<gl/GL.h>
#include<gl/GLU.h>

#include<stdio.h>
#include<stdlib.h>

#include<vector>

#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 256
#define S_EQUAL 0

#define WIN_INIT_X 100
#define WIN_INIT_Y 100

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define VK_F 0x46
#define VK_f 0x60

#define NR_POINT_COORDS 3
#define NR_TEXTURE_COORDS 2
#define NR_NORMAL_COORDS 3
#define NR_FACE_TOKENS 3

#define FOY_ANGLE 54
#define ZNEAR 0.1
#define ZFAR 200.0

#define VIEWPORT_BOTTOMLEFT_X 0
#define VIEWPORT_BOTTOMLEFT_Y 0

#define MONKEYHEAD_X_TRANSLATE 0.0f
#define MONKEYHEAD_Y_TRANSLATE 0.0f
#define MONKEYHEAD_Z_TRANSLATE -5.0f

#define MONKEYHEAD_X_SCALE_FACTOR 1.5f
#define MONKEYHEAD_Y_SCALE_FACTOR 1.5f
#define MONKEYHEAD_Z_SCALE_FACTOR 1.5f

#define START_ANGLE_POS 0.0f
#define END_ANGLE_POS 360.0f
#define MONKEYHEAD_ANGLE_INCREMENT 0.01f

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define ERRORBOX1(lpszErrorMessage, lpszCaption){\
												MessageBox((HWND)NULL,TEXT(lpszErrorMessage),TEXT(lpszCaption),MB_ICONERROR);\
												ExitProcess(EXIT_FAILURE);\
												}
#define ERRORBOX2(hWnd,lpszErrorMessage,lpszCaption){\
												MessageBox((HWND)NULL, TEXT(lpszErrorMessage), TEXT(lpszCaption),MB_ICONERROR);\
												DestroyWindow(hWnd);\
												}
												
LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

HWND g_hwnd = NULL;
HDC g_hdc = NULL;
HGLRC g_hrc = NULL;

DWORD g_dwStyle = NULL;
WINDOWPLACEMENT g_wpPrev;

bool g_bActiveWindow = false;
bool g_bEscapeKeyIsPressed = false;
bool g_bFullScreen = false;

GLfloat g_rotate;

std::vector<std::vector<float>> g_vertices;
std::vector<std::vector<float>> g_texture;
std::vector<std::vector<float>> g_normal;

std::vector<std::vector<int>> g_face_tri,g_face_texture,g_face_normals;

FILE *g_fp_meshfile = NULL;
FILE *g_fp_logfile = NULL;
char line[BUFFER_SIZE];

bool gbLightMode = false;

GLfloat light_ambient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat light_difused[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[] = {0.0f,0.0f,1.0f,1.0f};

GLfloat material_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_shinyness[] = {50.0f};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
		void initialize(void);
		void update(void);
		void display(void);
		void uninitialize(void);
		
		static TCHAR szAppName[] = TEXT("Mesh Loading Version 2");
		HWND hWnd = NULL;
		HBRUSH hBrush = NULL;
		HCURSOR hCursor = NULL;
		HICON hIcon = NULL;
		HICON hIconSm = NULL;
		
		bool bDone = false;
		
		WNDCLASSEX wndEx;
		MSG msg;
		ZeroMemory((void*)&wndEx, sizeof(WNDCLASSEX));
		ZeroMemory((void*)&msg,sizeof(MSG));
		
		hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		if(!hBrush)
		{
			ERRORBOX1("Error in getting stock object","GetStockObject Error");
		}
		
		hCursor = LoadCursor((HINSTANCE)NULL,IDC_ARROW);
		if(!hCursor)
		{
			ERRORBOX1("Error in loading a cursor","LoadCursor Error");
		}
		
		hIcon = LoadIcon((HINSTANCE)NULL,IDI_APPLICATION);
		if(!hIcon)
		{
			ERRORBOX1("Error in loading icon","LoadIcon Error");
		}
		hIconSm = LoadIcon((HINSTANCE)NULL,IDI_APPLICATION);
		if(!hIconSm)
		{
			ERRORBOX1("Error in loading icon","LoadIcon Error");
		}
		
		wndEx.cbClsExtra = 0;
		wndEx.cbWndExtra = 0;
		wndEx.cbSize = sizeof(WNDCLASSEX);
		wndEx.hbrBackground = hBrush;
		wndEx.hCursor = hCursor;
		wndEx.hIcon = hIcon;
		wndEx.hIconSm = hIconSm;
		wndEx.hInstance = hInstance;
		wndEx.lpfnWndProc = WndProc;
		wndEx.lpszClassName = szAppName;
		wndEx.lpszMenuName = NULL;
		wndEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		
		if(!RegisterClassEx(&wndEx))
		{
			ERRORBOX1("Error in registering a class","RegisterClassEx Error");
		}
		
		hWnd = CreateWindowEx(
								WS_EX_APPWINDOW,
								szAppName,
								szAppName,
								WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
								WIN_INIT_X,
								WIN_INIT_Y,
								WIN_WIDTH,
								WIN_HEIGHT,
								(HWND)NULL,
								(HMENU)NULL,
								hInstance,
								(LPVOID)NULL
							);
							
		if(!hWnd)
		{
			ERRORBOX1("Error in creating a window in memory","CreateWindowEx Error");
		}
		
		g_hwnd = hWnd;
		
		initialize();
		
		ShowWindow(hWnd,SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		
		//game loop
		while(bDone == false)
		{
			if(PeekMessage(&msg,(HWND)NULL,0,0,PM_REMOVE))
			{
				if(msg.message==WM_QUIT)
				{
						bDone = true;
				}else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}else
			{
				if(g_bActiveWindow==true)
				{
					if(g_bEscapeKeyIsPressed)
					{
						bDone = true;
					}else
					{
						update();
						display();
					}
				}
			}
		}
		uninitialize();
		return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	void resize(int,int);
	void ToggleFullScreen(void);
	void uninitialize(void);
	void toggleLightMode(void);
	
	switch(uMsg)
	{
		case WM_ACTIVATE:
				if(HIWORD(wParam)==0)
				{
					g_bActiveWindow = true;
				}else
				{
					g_bActiveWindow = false;
				}
			break;
			
		case WM_ERASEBKGND:
				return (0);
			break;
			
		case WM_SIZE:
				resize(LOWORD(lParam),HIWORD(lParam));
			break;
			
		case WM_KEYDOWN:
				switch(wParam)
				{
					case VK_ESCAPE:
							if(g_bEscapeKeyIsPressed == false)
							{
								g_bEscapeKeyIsPressed = true;
							}
						break;
						
					case VK_F:
					case VK_f:
							if(g_bFullScreen==false)
							{
								ToggleFullScreen();
								g_bFullScreen=true;
							}else
							{
								ToggleFullScreen();
								g_bFullScreen=false;
							}
						break;
					case 0x4C: //L
						toggleLightMode();
					break;
					default:
						break;
				}
			break;
		
		case WM_LBUTTONDOWN:
			
		break;
		
		case WM_CLOSE:
			uninitialize();
		break;
		
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
		break;
		
		default:
		
		break;
	}
	return (DefWindowProc(hWnd,uMsg,wParam,lParam));
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;
	
	if(g_bFullScreen==false)
	{
		g_dwStyle = GetWindowLong(g_hwnd,GWL_STYLE);
		if(g_dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if(GetWindowPlacement(g_hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hwnd,MONITORINFOF_PRIMARY),&mi))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(g_hwnd,HWND_TOP, mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,
								SWP_NOZORDER|SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
		}
	}else
	{
		SetWindowLong(g_hwnd,GWL_STYLE,g_dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hwnd, &g_wpPrev);
		SetWindowPos(g_hwnd,HWND_TOP,0,0,0,0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

void initialize(void)
{
	void resize(int, int);
	void uninitialize(void);
	void LoadMeshData(void);
	
	g_fp_logfile = fopen("monkeyheadloader.log","w");
	if(!g_fp_logfile)
	{
		uninitialize();
	}
	
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = -1;
	
	ZeroMemory((void*)&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	
	g_hdc = GetDC(g_hwnd);
	
	iPixelFormatIndex = ChoosePixelFormat(g_hdc, &pfd);
	if(iPixelFormatIndex==0)
	{
		ReleaseDC(g_hwnd, g_hdc);
		g_hdc=NULL;
	}
	
	if(SetPixelFormat(g_hdc,iPixelFormatIndex, &pfd)==false)
	{
		ReleaseDC(g_hwnd,g_hdc);
		g_hdc=NULL;
	}
	
	g_hrc=wglCreateContext(g_hdc);
	if(g_hrc==NULL)
	{
		ReleaseDC(g_hwnd, g_hdc);
		g_hdc=NULL;
	}
	
	if(wglMakeCurrent(g_hdc,g_hrc) == false)
	{
		wglDeleteContext(g_hrc);
		g_hrc = NULL;
		ReleaseDC(g_hwnd, g_hdc);
		g_hdc=NULL;
	}
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	LoadMeshData();
	
	glLightfv(GL_LIGHT0,GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE, light_difused);
	glLightfv(GL_LIGHT0,GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0,GL_POSITION, light_position);
	glMaterialfv(GL_FRONT,GL_SPECULAR,material_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,material_shinyness);
	glEnable(GL_LIGHT0);
	resize(WIN_WIDTH,WIN_HEIGHT);
}

void resize(int width,int height)
{
	if(height==0)
	{
		height=1;
	}
	glViewport(VIEWPORT_BOTTOMLEFT_X, VIEWPORT_BOTTOMLEFT_Y, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOY_ANGLE, (GLfloat)width/(GLfloat)height, ZNEAR, ZFAR);
}

void LoadMeshData(void)
{
	void uninitialize(void);
	
	g_fp_meshfile = fopen("MonkeyHead.OBJ","r");
	if(!g_fp_meshfile)
	{
		uninitialize();
	}
	
	char *sep_space = " ";
	char *sep_fslash = "/";
	
	char *first_token=NULL;
	char *token=NULL;
	
	char *face_tokens[NR_FACE_TOKENS];
	
	int nr_tokens;
	
	char *token_vertex_index=NULL;
	char *token_texture_index = NULL;
	char *token_normal_index = NULL;
	
	while(fgets(line,BUFFER_SIZE,g_fp_meshfile) != NULL)
	{
		first_token = strtok(line, sep_space);
		
		if(strcmp(first_token, "v")==S_EQUAL)
		{
			std::vector<float> vec_point_coord(NR_POINT_COORDS);
			for(int i=0;i!=NR_POINT_COORDS;i++)
			{
				vec_point_coord[i]=atof(strtok(NULL,sep_space));
			}
			g_vertices.push_back(vec_point_coord);
		}else if(strcmp(first_token,"vt")==S_EQUAL)
		{
			std::vector<float> vec_texture_coord(NR_TEXTURE_COORDS);
			for(int i=0;i!=NR_TEXTURE_COORDS;i++)
			{
				vec_texture_coord[i] = atof(strtok(NULL, sep_space));
			}
			g_texture.push_back(vec_texture_coord);
		}else if(strcmp(first_token,"vn")==S_EQUAL)
		{
			std::vector<float> vec_normal_coord(NR_NORMAL_COORDS);
			for(int i=0;i!=NR_NORMAL_COORDS;i++)
			{
				vec_normal_coord[i] = atof(strtok(NULL, sep_space));
			}
			g_normal.push_back(vec_normal_coord);
		}else if(strcmp(first_token,"f")==S_EQUAL)
		{
			std::vector<int> triangle_vertex_indices(3), texture_vertex_indices(3), normal_vertex_indices(3);
			
			memset((void*)face_tokens,0,NR_FACE_TOKENS);
			nr_tokens = 0;
			while(token=strtok(NULL, sep_space))
			{
				if(strlen(token)<3)
				{
					break;
				}
				face_tokens[nr_tokens]=token;
				nr_tokens++;
			}
			
			for(int i=0; i!=NR_FACE_TOKENS;++i)
			{
				token_vertex_index = strtok(face_tokens[i], sep_fslash);
				token_texture_index = strtok(NULL,sep_fslash);
				token_normal_index = strtok(NULL, sep_fslash);
				triangle_vertex_indices[i] = atoi(token_vertex_index);
				texture_vertex_indices[i] = atoi(token_texture_index);
				normal_vertex_indices[i] = atoi(token_normal_index);
			}
			g_face_tri.push_back(triangle_vertex_indices);
			g_face_texture.push_back(texture_vertex_indices);
			g_face_normals.push_back(normal_vertex_indices);
		}
		memset((void*)line, (int) '\0', BUFFER_SIZE);
	}
	fclose(g_fp_meshfile);
	g_fp_meshfile = NULL;
	
	fprintf(g_fp_logfile, "g_vertices:%llu g_texture:%llu g_normals:%llu g_face_tri:%llu\n",
			g_vertices.size(), g_texture.size(),g_normal.size(), g_face_tri.size());
}

void update(void)
{
	g_rotate = g_rotate + MONKEYHEAD_ANGLE_INCREMENT;
	
	if(g_rotate >= END_ANGLE_POS)
	{
			g_rotate = START_ANGLE_POS;
	}
}

void display(void)
{
	void uninitialize(void);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(MONKEYHEAD_X_TRANSLATE,MONKEYHEAD_Y_TRANSLATE,MONKEYHEAD_Z_TRANSLATE);
	glRotatef(g_rotate, 0.0f,1.0f,0.0f);
	glScalef(MONKEYHEAD_X_SCALE_FACTOR,MONKEYHEAD_Y_SCALE_FACTOR,MONKEYHEAD_Z_SCALE_FACTOR);
	glFrontFace(GL_CCW);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	for(int i=0;i!=g_face_tri.size();++i)
	{
		glBegin(GL_TRIANGLES);
			for(int j=0;j!=g_face_tri[i].size();j++)
			{
				int vi = g_face_tri[i][j]-1;
				glVertex3f(g_vertices[vi][0], g_vertices[vi][1], g_vertices[vi][2]);
			}
		glEnd();
	}
	SwapBuffers(g_hdc);
}

void uninitialize(void)
{
	if(g_bFullScreen==true)
	{
		g_dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);
		SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hwnd, &g_wpPrev);
		SetWindowPos(g_hwnd, HWND_TOP,0,0,0,0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
					);
		ShowCursor(TRUE);
	}
	wglMakeCurrent((HDC)NULL, (HGLRC)NULL);
	wglDeleteContext(g_hrc);
	g_hrc = (HGLRC)NULL;
	ReleaseDC(g_hwnd, g_hdc);
	g_hdc = (HDC) NULL;
	
	fclose(g_fp_logfile);
	g_fp_logfile = NULL;
	
	DestroyWindow(g_hwnd);
}

void toggleLightMode()
{
	if(gbLightMode==false)
	{
		glEnable(GL_LIGHTING);
		gbLightMode=true;
	}else
	{
		glDisable(GL_LIGHTING);
		gbLightMode=false;
	}
}