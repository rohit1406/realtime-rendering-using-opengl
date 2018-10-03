#include<windows.h>
#include<stdio.h> //for file I/O

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning ( disable: 4838)
#include "XNAMath\xnamath.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variable declaration
FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";

HWND ghwnd = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;

float gClearColor[4]; //RGBA
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_Pyramid_VertexBuffer_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_Pyramid_VertexBuffer_Color = NULL;

//rectangle vbos
ID3D11Buffer *gpID3D11Buffer_Cube_VertexBuffer_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_Cube_VertexBuffer_Color = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstanctBuffer = NULL;

ID3D11RasterizerState *gpRS;

//depth related changes
ID3D11DepthStencilView *gpID3D11DepthStencileView = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX gPerspectiveProjectionMatrix;

//rotation angle
float gfTriAngleRotation = 0.0f;
float gfSquareAngleRotation = 0.0f;
//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);
	void updateAngle(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("Direct3D11");
	bool bDone = false;

	//code
	//create log file
	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Can Not Be Created\nExitting..."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log File Is Successfully Opened.\n");
		fclose(gpFile);
	}

	//initialize WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//Register WNDCLASSEX structure
	RegisterClassEx(&wndclass);

	//create window
	hwnd = CreateWindow(szClassName,
		TEXT("Direct3D11 Window"),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//initialize D3D
	HRESULT hr;
	hr = initialize();

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "initialize() Failed. Exitting Now...\n");
		fclose(gpFile);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "initialize() Successed.\n");
		fclose(gpFile);
	}

	//message loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			//update angle
			updateAngle();
			//render
			display();

			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
			}
		}
	}

	//clean-up
	uninitialize();

	return ((int)msg.wParam);
}

/* WndProc: callback function */
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declarations
	HRESULT resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);

	//variable declaration
	HRESULT hr;

	//code
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0) //if 0, window is active
		{
			gbActiveWindow = true;
		}
		else //if non-zero, window is not active
		{
			gbActiveWindow = false;
		}
		break;

	case WM_ERASEBKGND:
		return(0);
		break;

	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
		

		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "resize() Failed.\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "resize() Succeeded.\n");
			fclose(gpFile);
		}
}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE: //case 27
			if (gbEscapeKeyIsPressed == false)
			{
				gbEscapeKeyIsPressed = true;
			}
			break;

		case 0x46: // 'f' or 'F'
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
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
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//variable declaration
	MONITORINFO mi;

	//code
	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
			ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

HRESULT initialize(void)
{
	//function declaration
	void uninitialize(void);
	HRESULT resize(int, int);

	//variable declarations
	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE, 
	};
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0; //default, lowest
	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1; //based upon d3dFeatureLevel_required

	//code
	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void *) &dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex=0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
		NULL,
			d3dDriverType,
			NULL,
			createDeviceFlags,
			&d3dFeatureLevel_required,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDesc,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevel_acquired,
			&gpID3D11DeviceContext
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() Succeeded.\n");
		fprintf_s(gpFile, "The Chosen Driver Is Of ");

		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "HardWare Type.\n");
		}else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "Warp Type.\n");
		}else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference Type.\n");
		}else
		{
			fprintf_s(gpFile, "Unknown Type.\n");
		}

		fprintf_s(gpFile, "The Supported Highest Feature Level Is ");

		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0.\n");
		}else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10_1.\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10_0.\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown Type.\n");
		}
		fclose(gpFile);
	}

	//initialize shaders, input layouts, constant buffers etc
	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
		"float4x4 worldViewProjectionMatrix;"\
		"}"\
		"struct vertex_output"\
		"{"\
		"float4 position : SV_POSITION;"\
		"float4 color : COLOR; "\
		"};"\
		"vertex_output main(float3 pos : POSITION, float3 col : COLOR)"\
		"{"\
		"vertex_output output;"\
		"output.position = mul(worldViewProjectionMatrix, float4(pos,1.0));"\
		"output.color = float4(col,1.0);"\
		"return (output);"\
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode)+1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Vertex Shader : %s.\n", (char *)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return (hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Vertex Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);

	if (FAILED(hr))
	{
		
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Failed.\n");
			fclose(gpFile);
			return (hr);
		
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);


	const char *pixelShaderSourceCode =
		"float4 main(float4 position: SV_POSITION, float4 color : COLOR) : SV_TARGET"\
		"{"\
		"float4 output_color;"\
		"output_color = color;"\
		"return (output_color);"\
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader : %s.\n", (char *)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return (hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Pixel Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);

	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;

	//triangle data
	float pyramid_vertices[] =
	{
		/*//triangle of front side
		0.0f, 1.0f, 0.0f, //front-top
		-1.0f, -1.0f, 1.0f, //front-left
		1.0f, -1.0f, 1.0f, //front-right

		//triangle of right side
		0.0f, 1.0f, 0.0f, //right-top
		1.0f, -1.0f, 1.0f, //right-left
		1.0f, -1.0f, -1.0f, //right-right

		//triangle of back side
		0.0f, 1.0f, 0.0f, //back-top
		1.0f, -1.0f, -1.0f, //back-right
		1.0f, -1.0f, -1.0f, //back-left

		//triangle of left side
		0.0f, 1.0f, 0.0f, //left-top
		-1.0f, -1.0f, -1.0f, //left-left
		1.0f, -1.0f, 1.0f //left-right*/
		//front face
		0.0f,1.0f,0.0f, //apex
		1.0f,-1.0f,-1.0f, //right
		-1.0f,-1.0f,-1.0f, //left
		//right face
		0.0f,1.0f,0.0f, //apex
		1.0f,-1.0f,1.0f,//right
		1.0f,-1.0f,-1.0f,//left
		//back
		0.0f,1.0f,0.0f, //apex
		-1.0f,-1.0f,1.0f,//right
		1.0f,-1.0f,1.0f,//left
		//left
		0.0f,1.0f,0.0f, //apex
		-1.0f,-1.0f,-1.0f,//right
		-1.0f,-1.0f,1.0f,//left

							//DX Cloclwise winding order
	};

	float pyramid_color[] =
	{
		/*//triangle of front side
		1.0f, 0.0f, 0.0f, //R top
		0.0f, 1.0f, 0.0f, //G
		0.0f, 0.0f, 1.0f, //B

		//triangle of right side
		1.0f, 0.0f, 0.0f, //R top
		0.0f, 1.0f, 0.0f, //G
		0.0f, 0.0f, 1.0f, //B

		//triangle of back side
		1.0f, 0.0f, 0.0f, //R top
		0.0f, 1.0f, 0.0f, //G
		0.0f, 0.0f, 1.0f, //B
		
		//triangle of left size
		1.0f, 0.0f, 0.0f, //R top
		0.0f, 1.0f, 0.0f, //G
		0.0f, 0.0f, 1.0f //B*/
		//front
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		//0.0f,1.0f,0.0f,
		//0.0f,0.0f,1.0f,
		//right
		//1.0f,0.0f,0.0f,
		//0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		//back
		//1.0f,0.0f,0.0f,
		//0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		//left
		//1.0f,0.0f,0.0f,
		//0.0f,0.0f,1.0f,
		//0.0f,1.0f,0.0f,      
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
	};

	//rectangle data
	float cube_vertices[] =
	{
		//SIDE 1 (TOP)
		//triangle 1
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		//triangle 2
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,

		//SIDE 2 (BOTTOM)
		//triangle 1
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		//trinalge 2
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f,1.0f,

		//SIDE 3 (FRONT)
		//triangle 1
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		//triangle 2
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		//SIDE 4 (BACK)
		//triangle 1
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		//triangle 2
		-1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		//SIDE 5 (LEFT)
		//triangle 1
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		//triangle 2
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		//SIDE 6 (RIGHT)
		//triangle1
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		//triangle 2
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f
	};

	float cube_colors[]
	{
		//SIDE 1 (TOP) - RED
		//triangle 1 of side 1
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		//triangle 2 of side 1
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		//SIDE 2 (BOTTOM) - GREEN
		//triangle 1 of side 2
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		//triangle 2 of side 2
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		//SIDE 3 (FRONT) - BLUE
		//triangle 1 of side 3
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		//triangle 2 of side 3
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		//SIDE 4 (BACK) - CYAN
		//triangle 1 of side 4
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		//triangle 2 of side 4
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		//SIDE 5 (LEFT) - MAGENTA
		//triangle 1 of side 5
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		//triangle 2 of side 5
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,

		//SIDE 6 (RIGHT) - YELLOW
		//triangle 1 of side 6
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		//triangle 2 of side 6
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	//create vertex position buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(pyramid_vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL,
		&gpID3D11Buffer_Pyramid_VertexBuffer_Position);

	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_Pyramid_VertexBuffer_Position, NULL, D3D11_MAP_WRITE_DISCARD,
		NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, pyramid_vertices, sizeof(pyramid_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_Pyramid_VertexBuffer_Position, NULL);

	//create vertex color buffer
	D3D11_BUFFER_DESC bufferDescCol;
	ZeroMemory(&bufferDescCol, sizeof(D3D11_BUFFER_DESC));
	bufferDescCol.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescCol.ByteWidth = sizeof(float) * ARRAYSIZE(pyramid_color);
	bufferDescCol.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescCol.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescCol, NULL, &gpID3D11Buffer_Pyramid_VertexBuffer_Color);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed For Color Buffer.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Color Buffer.\n");
		fclose(gpFile);
	}

	//copy colors into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubResourceForColor;
	ZeroMemory(&mappedSubResourceForColor, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_Pyramid_VertexBuffer_Color, NULL,
		D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResourceForColor);
	memcpy(mappedSubResourceForColor.pData, pyramid_color, sizeof(pyramid_color));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_Pyramid_VertexBuffer_Color, NULL);



	//********************CUBE***************
	//create vertex position buffer for Rectangle
	D3D11_BUFFER_DESC bufferDescRect;
	ZeroMemory(&bufferDescRect, sizeof(D3D11_BUFFER_DESC));
	bufferDescRect.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescRect.ByteWidth = sizeof(float) * ARRAYSIZE(cube_vertices);
	bufferDescRect.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescRect.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescRect, 
		NULL,
		&gpID3D11Buffer_Cube_VertexBuffer_Position);

	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer of Rectangle.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer of Rectangle.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubResourceRect;
	ZeroMemory(&mappedSubResourceRect, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_Cube_VertexBuffer_Position,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubResourceRect);
	memcpy(mappedSubResourceRect.pData, cube_vertices, sizeof(cube_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_Cube_VertexBuffer_Position, NULL);

	//create vertex color buffer for rectangle
	D3D11_BUFFER_DESC bufferDescColRect;
	ZeroMemory(&bufferDescColRect, sizeof(D3D11_BUFFER_DESC));
	bufferDescColRect.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescColRect.ByteWidth = sizeof(float) * ARRAYSIZE(cube_colors);
	bufferDescColRect.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescColRect.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescColRect,
		NULL,
		&gpID3D11Buffer_Cube_VertexBuffer_Color);

	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed For Vertex Color Buffer of Rectangle.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Color Buffer of Rectangle.\n");
		fclose(gpFile);
	}

	//copy colors into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubResourceForColorRect;
	ZeroMemory(&mappedSubResourceForColorRect, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_Cube_VertexBuffer_Color,
		NULL,
		D3D11_MAP_WRITE_DISCARD, 
		NULL,
		&mappedSubResourceForColorRect);
	memcpy(mappedSubResourceForColorRect.pData, cube_colors, sizeof(cube_colors));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_Cube_VertexBuffer_Color, NULL);

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	ZeroMemory(inputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC));
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "COLOR";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;


	hr = gpID3D11Device->CreateInputLayout(inputElementDesc, 2, pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(), &gpID3D11InputLayout);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Failed.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	//define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstanctBuffer);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed For Constant Buffer.\n");
		fclose(gpFile);
		return (hr);

	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Constant Buffer.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->VSSetConstantBuffers(0,1,&gpID3D11Buffer_ConstanctBuffer);

	//d3d clear color (blue)
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.1f;
	gClearColor[2] = 0.2f;
	gClearColor[3] = 0.3f;

	//set projection matrix to identity matrix
	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	//cull face disable
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
	rd.AntialiasedLineEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0;
	rd.CullMode = D3D11_CULL_NONE;
	rd.DepthClipEnable = TRUE;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.FrontCounterClockwise = FALSE;
	rd.ScissorEnable = FALSE;

	gpID3D11Device->CreateRasterizerState(&rd, &gpRS);
	gpID3D11DeviceContext->RSSetState(gpRS);

	//call resize for first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Succeeded.\n");
		fclose(gpFile);
	}
	return(S_OK);
}

HRESULT resize(int width, int height)
{
	//code
	HRESULT  hr = S_OK;

	//free any size-dependent resources
	if (gpID3D11DepthStencileView)
	{
		gpID3D11DepthStencileView->Release();
		gpID3D11DepthStencileView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}


	//resize swap chain buffers accordingly
	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//again get back buffer from swap chain
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);

	//again get render target view from d3d11 device using above back buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer,
		NULL,
		&gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() Succeeded.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//set render target view as render target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

	//for depth
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT) width;
	textureDesc.Height = (UINT) height;
	textureDesc.ArraySize = 1; //since we are not considering it as a texture, but as a depth
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1; // 1 to 4 aste
	textureDesc.SampleDesc.Quality = 0; //4 mhanun 1, 1 asel tar 0
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//create 2d texture
	ID3D11Texture2D *pID3D11Texture2D = NULL;
	hr = gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateTexture2D() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateTexture2D() Succeeded.\n");
		fclose(gpFile);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencileViewDesc;
	ZeroMemory(&depthStencileViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencileViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencileViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; //texture 2d with multisampling
	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D, &depthStencileViewDesc, &gpID3D11DepthStencileView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencileView);

	/*if (pID3D11Texture2D)
	{
		pID3D11Texture2D->Release();
		pID3D11Texture2D = NULL;
	}*/
	//set viewport
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	//set orthographic matrix
	/*if (width <= height)
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(-100.0f, 100.0f, -100.0f * ((float)height / (float)width),
			100.0f * ((float)height / (float)width), -100.0f, 100.0f);
	}
	else
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(
			-100.0f * ((float)width/ (float)height), 
			100.0f *((float)width/ (float)height),
			-100.0f, 100.0f, -100.0f, 100.0f);
	}*/

	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(45.0f, (float) width/ (float) height, 0.1f, 100.0f);
	return (hr);
}

void display(void)
{
	//code
	//clear depth stencile view
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencileView, D3D11_CLEAR_DEPTH, 1.0, 0);
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	//select which vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;

	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_Pyramid_VertexBuffer_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_Pyramid_VertexBuffer_Color, &stride, &offset);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation is concerned with world matrix transformation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX modelMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();

	worldMatrix = XMMatrixTranslation(-1.5f, 0.0f, 6.0f);
	
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(gfTriAngleRotation));
	worldMatrix =  rotationMatrix * worldMatrix;
	//final world view projection matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	//load the data into the constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstanctBuffer, 0, NULL, &constantBuffer, 0,0);

	//draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(12, 0);
	
	/*******Draw Triangle Ends********/

	/***************Draw Cube Starts*******************/
	//select which vertex buffer to display
	stride = sizeof(float) * 3;
	offset = 0;

	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_Cube_VertexBuffer_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_Cube_VertexBuffer_Color, &stride, &offset);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//draw cube
	worldMatrix = XMMatrixIdentity();
	modelMatrix = XMMatrixIdentity();
	wvpMatrix = XMMatrixIdentity();
	rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixIdentity();

	worldMatrix = XMMatrixTranslation(1.5f, 0.0f, 6.0f);
	scaleMatrix = XMMatrixScaling(0.75f, 0.75f, 0.75f);
	rotationMatrix = rotationMatrix *XMMatrixRotationX(XMConvertToRadians(gfSquareAngleRotation));
	rotationMatrix = rotationMatrix * XMMatrixRotationY(XMConvertToRadians(gfSquareAngleRotation));
	rotationMatrix = rotationMatrix * XMMatrixRotationZ(XMConvertToRadians(gfSquareAngleRotation));
	worldMatrix = scaleMatrix * rotationMatrix * worldMatrix;
	wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	//load the data into the constat buffer
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstanctBuffer, 0, NULL, &constantBuffer, 0, 0);

	//draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		0 //start vertex location
	);

	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		6 //start vertex location
	);

	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		12 //start vertex location
	);
	
	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		18 //start vertex location
	);

	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		24 //start vertex location
	);

	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		30 //start vertex location
	);

	gpID3D11DeviceContext->Draw(
		6,  //vertex count
		36 //start vertex location
	);
	//translate


	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);
}

void uninitialize(void)
{
	//code
	if (gpID3D11Buffer_ConstanctBuffer)
	{
		gpID3D11Buffer_ConstanctBuffer->Release();
		gpID3D11Buffer_ConstanctBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}
	if (gpID3D11Buffer_Pyramid_VertexBuffer_Position)
	{
		gpID3D11Buffer_Pyramid_VertexBuffer_Position->Release();
		gpID3D11Buffer_Pyramid_VertexBuffer_Position = NULL;
	}
	if (gpID3D11Buffer_Pyramid_VertexBuffer_Color)
	{
		gpID3D11Buffer_Pyramid_VertexBuffer_Color->Release();
		gpID3D11Buffer_Pyramid_VertexBuffer_Color = NULL;
	}

	if (gpID3D11Buffer_Cube_VertexBuffer_Position)
	{
		gpID3D11Buffer_Cube_VertexBuffer_Position->Release();
		gpID3D11Buffer_Cube_VertexBuffer_Position = NULL;
	}
	if (gpID3D11Buffer_Cube_VertexBuffer_Color)
	{
		gpID3D11Buffer_Cube_VertexBuffer_Color->Release();
		gpID3D11Buffer_Cube_VertexBuffer_Color = NULL;
	}
	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpID3D11DepthStencileView)
	{
		gpID3D11DepthStencileView->Release();
		gpID3D11DepthStencileView = NULL;
	}
	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpRS)
	{
		gpRS->Release();
		gpRS = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "uninitialize() Succeeded.\n");
		fprintf_s(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
	}
}

void updateAngle()
{
	gfTriAngleRotation += 0.01f;

	if (gfTriAngleRotation >= 360.0f)
	{
		gfTriAngleRotation = 0.0f;
	}

	//update square angle
	gfSquareAngleRotation += 0.01f;

	if (gfSquareAngleRotation >= 360.0f)
	{
		gfSquareAngleRotation = 0.0f;
	}
}