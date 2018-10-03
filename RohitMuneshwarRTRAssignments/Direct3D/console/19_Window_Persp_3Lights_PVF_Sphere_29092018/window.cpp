#include<windows.h>
#include<stdio.h> //for file I/O

#include<d3d11.h>
#include<d3dcompiler.h>
#include "Sphere.h"

#pragma warning ( disable: 4838)
#include "XNAMath\xnamath.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment (lib, "Sphere.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variable declaration
FILE *gpFile = NULL;

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

//rectangle vbos
ID3D11Buffer *gpID3D11Buffer_Sphere_VertexBuffer_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_Sphere_VertexBuffer_Normals = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstanctBuffer = NULL;

ID3D11Buffer *gpID3D11Buffer_IndexBuffer = NULL;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements;
unsigned int gNumVertices;


float light0_ambient[] = { 1.0f,0.0f,0.0f,1.0f };
float light0_diffuse[] = { 1.0f,0.0f,0.0f,1.0f };
float light0_specular[] = { 1.0f,0.0f,0.0f,1.0f };
float light0_position[] = { 0.0f,0.0f,0.0f,1.0f };

float light1_ambient[] = { 0.0f,1.0f,0.0f,1.0f };
float light1_diffuse[] = { 0.0f,1.0f,0.0f,1.0f };
float light1_specular[] = { 0.0f,1.0f,0.0f,1.0f };
float light1_position[] = { 0.0f,0.0f,0.0f,1.0f };

float light2_ambient[] = { 0.0f,0.0f,1.0f,1.0f };
float light2_diffuse[] = { 0.0f,0.0f,1.0f,1.0f };
float light2_specular[] = { 0.0f,0.0f,1.0f,1.0f };
float light2_position[] = { 0.0f,0.0f,0.0f,1.0f };

float material_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
float material_diffuse[] = { 1.0f,1.0f,1.0f,1.0f };
float material_specular[] = { 1.0f,1.0f,1.0f,1.0f };

float material_shininess = 50.0;
//As we need in setting input layout , it will relase after setting input layout
ID3DBlob *pID3DBlob_vertexShaderCode = NULL;

ID3D11RasterizerState *gpRS;

//depth related changes
ID3D11DepthStencilView *gpID3D11DepthStencileView = NULL;

char gszLogFileName[] = "Log.txt";
void log(const char* logMessage);

struct CBUFFER {
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR la0;
	XMVECTOR ld0;
	XMVECTOR ls0;

	XMVECTOR la1;
	XMVECTOR ld1;
	XMVECTOR ls1;

	XMVECTOR la2;
	XMVECTOR ld2;
	XMVECTOR ls2;

	XMVECTOR ka;
	XMVECTOR kd;
	XMVECTOR ks;
	float materialShininess;

	XMVECTOR lightPosition0;
	XMVECTOR lightPosition1;
	XMVECTOR lightPosition2;
	unsigned int lightToggle;
};

XMMATRIX gPerspectiveProjectionMatrix;

//rotation angle
float angleRotate = 0.0f;
unsigned int gAnimationToggle, gLightToggle;
unsigned int gbShaderToggle=0;

//need in setting input layout. will relase after setting input layout
//ID3DBlob *pID3DBlob_vertexShaderCode = NULL;

HRESULT hr;



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
		case 0x41:
			gAnimationToggle++;
			if (gAnimationToggle > 1)
				gAnimationToggle = 0;
			break;
		case 0x4C:
			gLightToggle++;
			if (gLightToggle > 1)
				gLightToggle = 0;

			if (gpFile)
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "Light Toggeled - %u.\n", gLightToggle);
				fclose(gpFile);
			}
			break;
		case 0x53: //S key
			gbShaderToggle++;
			if (gbShaderToggle > 1)
				gbShaderToggle = 0;
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
	HRESULT initVertexShader(void);
	HRESULT initPixelShader(void);
	void initSphereVBOAndData(void);
	HRESULT createAndSetInputLayout(void);
	HRESULT createAndSetConstantBuffers(void);
	void uninitialize(void);
	HRESULT resize(int, int);

	//variable declarations
	
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
	
	initVertexShader();
	initPixelShader();

	initSphereVBOAndData();

	createAndSetInputLayout();

	createAndSetConstantBuffers();

	//d3d clear color (blue)
	gClearColor[0] = 0.1f;
	gClearColor[1] = 0.2f;
	gClearColor[2] = 0.3f;
	gClearColor[3] = 1.0f;

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

	gAnimationToggle = 0;
	gLightToggle = 0;
	gbShaderToggle = 0;
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
HRESULT initVertexShader(void)
{
	const char* pvVertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
		"float4x4 worldMatrix; "\
		"float4x4 viewMatrix; "\
		"float4x4 projectionMatrix; "\
		"float4 la0; "\
		"float4 ld0; "\
		"float4 ls0; "\

		"float4 la1; "\
		"float4 ld1; "\
		"float4 ls1; "\

		"float4 la2; "\
		"float4 ld2; "\
		"float4 ls2; "\

		"float4 ka; "\
		"float4 kd; "\
		"float4 ks; "\

		"float materialShininess;"\
		"float4 lightPosition0; "\
		"float4 lightPosition1; "\
		"float4 lightPosition2; "\
		"uint lightToggle;"\
		"}"\

		"struct VertexOut {"\
		"float4 position: SV_POSITION;"\
		"float4 phong_ads_color: COLOR;"\
		"};"\


		"struct VertexIN{"\
		"float4 pos: POSITION;"\
		"float4 normal: NORMAL;"\
		"};"\

		"VertexOut main(VertexIN vin)"\
		"{"\
		"VertexOut output;"\
		"if(lightToggle==1)"\
		"{"\
		"float4 eyeCoordinates = mul(worldMatrix,vin.pos); "\
		"eyeCoordinates = mul(viewMatrix,eyeCoordinates); "\
		"float3 tnorm = normalize(mul((float3x3)mul(worldMatrix,viewMatrix),(float3)vin.normal)); "\
		"float3 lightDirectionSource0 = (float3)normalize(lightPosition0 - eyeCoordinates); "\
		"float3 lightDirectionSource1 = (float3)normalize(lightPosition1 - eyeCoordinates); "\
		"float3 lightDirectionSource2 = (float3)normalize(lightPosition2 - eyeCoordinates); "\

		"float tn_dot_ld0 = max(dot(tnorm,lightDirectionSource0),0.0); "\
		"float tn_dot_ld1 = max(dot(tnorm,lightDirectionSource1),0.0); "\
		"float tn_dot_ld2 = max(dot(tnorm,lightDirectionSource2),0.0); "\

		"float3 viewerVector = normalize(-eyeCoordinates.xyz); "\

		"float3 ambient0 = la0 * ka ;"\
		"float3 diffuse0 = ld0 * kd * tn_dot_ld0 ;"\
		"float3 reflectionVector0 = reflect(-lightDirectionSource0, tnorm); "\
		"float3 specular0 = ls0 * ks * pow(max(dot(reflectionVector0,viewerVector),0.0),materialShininess); "\

		"float3 ambient1 = la1 * ka ;"\
		"float3 diffuse1 = ld1 * kd * tn_dot_ld1 ;"\
		"float3 reflectionVector1 = reflect(-lightDirectionSource1, tnorm); "\
		"float3 specular1 = ls1 * ks * pow(max(dot(reflectionVector1,viewerVector),0.0),materialShininess); "\

		"float3 ambient2 = la2 * ka ;"\
		"float3 diffuse2 = ld2 * kd * tn_dot_ld1 ;"\
		"float3 reflectionVector2 = reflect(-lightDirectionSource2, tnorm); "\
		"float3 specular2 = ls2 * ks * pow(max(dot(reflectionVector2,viewerVector),0.0),materialShininess); "\

		"output.phong_ads_color = float4(ambient0 + diffuse0 + specular0 + ambient1 + diffuse1 + specular1 + ambient2 + diffuse2 + specular2,1.0);"\
		"} else {"\
		"output.phong_ads_color = float4(1.0,1.0,1.0,1.0);"\
		"}"\
		"output.position = mul(worldMatrix,vin.pos); "\
		"output.position = mul(viewMatrix,output.position); "\
		"output.position = mul(projectionMatrix,output.position );"\
		"return output;"\
		"}";

	const char* ppVertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
		"float4x4 worldMatrix; "\
		"float4x4 viewMatrix; "\
		"float4x4 projectionMatrix; "\
		"float4 la0; "\
		"float4 ld0; "\
		"float4 ls0; "\

		"float4 la1; "\
		"float4 ld1; "\
		"float4 ls1; "\

		"float4 la2; "\
		"float4 ld2; "\
		"float4 ls2; "\

		"float4 ka; "\
		"float4 kd; "\
		"float4 ks; "\

		"float materialShininess;"\
		"float4 lightPosition0; "\
		"float4 lightPosition1; "\
		"float4 lightPosition2; "\
		"uint lightToggle;"\
		"}"\

		"struct VertexOut {"\
		"float4 position: SV_POSITION;"\
		"float3 transformed_normal: NORMAL0;"\
		"float3 light_direction0: NORMAL1;"\
		"float3 light_direction1: NORMAL2;"\
		"float3 light_direction2: NORMAL3;"\
		"float3 viewer_vector: NORMAL4;"\
		"};"\

		"struct VertexIN{"\
		"float4 pos: POSITION;"\
		"float4 normal: NORMAL;"\
		"};"\

		"VertexOut main(VertexIN vin)"\
		"{"\
		"VertexOut output;"\
		"if(lightToggle==1)"\
		"{"\
		"float4 eyeCoordinates = mul(worldMatrix,vin.pos); "\
		"eyeCoordinates = mul(viewMatrix,eyeCoordinates); "\
		"output.transformed_normal = mul((float3x3)mul(worldMatrix,viewMatrix),(float3)vin.normal); "\
		"output.light_direction0 = (float3)(lightPosition0 - eyeCoordinates); "\
		"output.light_direction1 = (float3)(lightPosition1 - eyeCoordinates); "\
		"output.light_direction2 = (float3)(lightPosition2 - eyeCoordinates); "\
		"output.viewer_vector = (-eyeCoordinates.xyz); "\
		"}"\
		"output.position = mul(worldMatrix,vin.pos); "\
		"output.position = mul(viewMatrix,output.position); "\
		"output.position = mul(projectionMatrix,output.position );"\
		"return output;"\
		"}";

	ID3DBlob *pID3DBlob_error = NULL;
	HRESULT hr;
	if (gbShaderToggle == 0) {
		hr = D3DCompile(
			pvVertexShaderSourceCode,//psrcData
			lstrlenA(pvVertexShaderSourceCode) + 1,//size
			"VS",//pSourceName
			NULL,//d3d_shader_macro if any
			D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
			"main",//entry point
			"vs_5_0",//pTarget
			0,//flags1
			0,//flags2
			&pID3DBlob_vertexShaderCode,//ppCode
			&pID3DBlob_error//perrormessage
		);
	}
	else {

		hr = D3DCompile(
			ppVertexShaderSourceCode,//psrcData
			lstrlenA(ppVertexShaderSourceCode) + 1,//size
			"VS",//pSourceName
			NULL,//d3d_shader_macro if any
			D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
			"main",//entry point
			"vs_5_0",//pTarget
			0,//flags1
			0,//flags2
			&pID3DBlob_vertexShaderCode,//ppCode
			&pID3DBlob_error//perrormessage
		);
	}


	if (FAILED(hr))
	{
		if (pID3DBlob_error != NULL)
		{
			log("Failed: D3DCompile for vertex shader : ");
			log((char*)pID3DBlob_error->GetBufferPointer());
			log("\n\n");
			pID3DBlob_error->Release();
			pID3DBlob_error = NULL;
			return hr;
		}
	}
	else {
		log("Success: D3DCompile for vertex shader.\n");
	}

	hr = gpID3D11Device->CreateVertexShader(
		pID3DBlob_vertexShaderCode->GetBufferPointer(),//pShaderByteCode
		pID3DBlob_vertexShaderCode->GetBufferSize(), //bytecode length
		NULL,//pClassLinkage (if u have created uniform which is common accross shaders)
		&gpID3D11VertexShader //ppVertexShader
	);

	if (FAILED(hr))
		log("Failed: gpID3D11Device->CreateVertexShader.\n");
	else
		log("Success: gpID3D11Device->CreateVertexShader.\n");

	gpID3D11DeviceContext->VSSetShader(
		gpID3D11VertexShader,//pVertexShader
		0,//ppClassInstances
		0//numClassInstances
	);
}

HRESULT initPixelShader(void)
{
	const char* pvPixelShaderSourceCode =

		"struct VertexOut{"\
		"float4 pos: SV_POSITION;"\
		"float4 phong_ads_color: COLOR;"\
		"};"\

		"float4 main(VertexOut vIn) : SV_TARGET"\
		"{"\
		"return vIn.phong_ads_color;"\
		"}";

	const char* ppPixelShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
		"float4x4 worldMatrix; "\
		"float4x4 viewMatrix; "\
		"float4x4 projectionMatrix; "\
		"float4 la0; "\
		"float4 ld0; "\
		"float4 ls0; "\

		"float4 la1; "\
		"float4 ld1; "\
		"float4 ls1; "\

		"float4 la2; "\
		"float4 ld2; "\
		"float4 ls2; "\

		"float4 ka; "\
		"float4 kd; "\
		"float4 ks; "\

		"float materialShininess;"\
		"float4 lightPosition0; "\
		"float4 lightPosition1; "\
		"float4 lightPosition2; "\
		"uint lightToggle;"\
		"}"\

		"struct VertexOut {"\
		"float4 position: SV_POSITION;"\
		"float3 transformed_normal: NORMAL0;"\
		"float3 light_direction0: NORMAL1;"\
		"float3 light_direction1: NORMAL2;"\
		"float3 light_direction2: NORMAL3;"\
		"float3 viewer_vector: NORMAL4;"\
		"};"\

		"float4 main(VertexOut vIn) : SV_TARGET"\
		"{"\
		"float4 phong_ads_color;"\

		"if(lightToggle==1)"\
		"{"\
		"float3 normalized_tnorm = normalize(vIn.transformed_normal); "\
		"float3 normalized_light_direction0 = normalize(vIn.light_direction0); "\
		"float3 normalized_light_direction1 = normalize(vIn.light_direction1); "\
		"float3 normalized_light_direction2 = normalize(vIn.light_direction2); "\
		"float3 normalized_viewer_vector = normalize(vIn.viewer_vector); "\

		"float tn_dot_ld0 = max(dot(normalized_tnorm,normalized_light_direction0),0.0); "\
		"float4 ambient0 = la0 * ka ;"\
		"float4 diffuse0 = ld0 * kd * tn_dot_ld0 ;"\
		"float3 reflectionVector0 = reflect(-normalized_light_direction0, normalized_tnorm); "\
		"float4 specular0 = ls0 * ks * pow(max(dot(reflectionVector0,normalized_viewer_vector),0.0),materialShininess); "\

		"float tn_dot_ld1 = max(dot(normalized_tnorm,normalized_light_direction1),0.0); "\
		"float4 ambient1 = la1 * ka ;"\
		"float4 diffuse1 = ld1 * kd * tn_dot_ld1 ;"\
		"float3 reflectionVector1 = reflect(-normalized_light_direction1, normalized_tnorm); "\
		"float4 specular1 = ls1 * ks * pow(max(dot(reflectionVector1,normalized_viewer_vector),0.0),materialShininess); "\

		"float tn_dot_ld2 = max(dot(normalized_tnorm,normalized_light_direction2),0.0); "\
		"float4 ambient2 = la2 * ka ;"\
		"float4 diffuse2 = ld2 * kd * tn_dot_ld1 ;"\
		"float3 reflectionVector2 = reflect(-normalized_light_direction2, normalized_tnorm); "\
		"float4 specular2 = ls2 * ks * pow(max(dot(reflectionVector2,normalized_viewer_vector),0.0),materialShininess); "\

		"phong_ads_color = ambient0 + diffuse0 + specular0 +ambient1 + diffuse1 + specular1 +ambient2 + diffuse2 + specular2;"\
		"} else {"\
		"phong_ads_color = float4(1.0,1.0,1.0,1.0);"\
		"}"\
		"return phong_ads_color;"\
		"}";

	ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
	ID3DBlob *pID3DBlob_error = NULL;
	HRESULT hr;

	if (gbShaderToggle == 0) {
		hr = D3DCompile(
			pvPixelShaderSourceCode,//psrcData
			lstrlenA(pvPixelShaderSourceCode) + 1,//size
			"PS",//pSourceName
			NULL,//d3d_shader_macro if any
			D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
			"main",//entry point
			"ps_5_0",//pTarget
			0,//flags1
			0,//flags2
			&pID3DBlob_pixelShaderCode,//ppCode
			&pID3DBlob_error//perrormessage
		);
	}
	else {
		hr = D3DCompile(
			ppPixelShaderSourceCode,//psrcData
			lstrlenA(ppPixelShaderSourceCode) + 1,//size
			"PS",//pSourceName
			NULL,//d3d_shader_macro if any
			D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
			"main",//entry point
			"ps_5_0",//pTarget
			0,//flags1
			0,//flags2
			&pID3DBlob_pixelShaderCode,//ppCode
			&pID3DBlob_error//perrormessage
		);
	}


	if (FAILED(hr))
	{
		if (pID3DBlob_error != NULL)
		{
			log("Failed: D3DCompile for pixel shader : ");
			log((char*)pID3DBlob_error->GetBufferPointer());
			log("\n\n");
			pID3DBlob_error->Release();
			pID3DBlob_error = NULL;
			return hr;
		}
	}
	else {
		log("Success: D3DCompile for pixel shader.\n");
	}

	hr = gpID3D11Device->CreatePixelShader(
		pID3DBlob_pixelShaderCode->GetBufferPointer(),//pShaderByteCode
		pID3DBlob_pixelShaderCode->GetBufferSize(), //bytecode length
		NULL,//pClassLinkage (if u have created uniform which is common accross shaders)
		&gpID3D11PixelShader //ppVertexShader
	);

	if (FAILED(hr))
		log("Failed: gpID3D11Device->CreateVertexShader.\n");
	else
		log("Success: gpID3D11Device->CreateVertexShader.\n");

	gpID3D11DeviceContext->PSSetShader(
		gpID3D11PixelShader,//pVertexShader
		0,//ppClassInstances
		0//numClassInstances
	);

	pID3DBlob_pixelShaderCode->Release();
	pID3DBlob_pixelShaderCode = NULL;

}

void initSphereVBOAndData() {
	//********************************Sphere DATA BINDING***********************************

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumElements = getNumberOfSphereElements();
	gNumVertices = getNumberOfSphereVertices();

	//vertex
	//create vertex buffer //similar to vbo in opengl
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(sphere_vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = gpID3D11Device->CreateBuffer(
		&bufferDesc,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_Sphere_VertexBuffer_Position//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for Sphere vertices position.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for Sphere vertices position.\n");
	}

	//now copy vertices into above created buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(
		gpID3D11Buffer_Sphere_VertexBuffer_Position,//pResource
		NULL,//SubResource
		D3D11_MAP_WRITE_DISCARD,//MapType
		NULL,//MAPFlags
		&mappedSubresource//pMappedResource
	);
	memcpy(
		mappedSubresource.pData,//dest
		sphere_vertices,//source
		sizeof(sphere_vertices)//size
	);
	gpID3D11DeviceContext->Unmap(
		gpID3D11Buffer_Sphere_VertexBuffer_Position,//pResource
		NULL//subresource
	);

	//Normal
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(sphere_normals);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_Sphere_VertexBuffer_Normals//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for Sphere vertices normal.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for Sphere vertices normal.\n");
	}


	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(
		gpID3D11Buffer_Sphere_VertexBuffer_Normals,//pResource
		NULL,//SubResource
		D3D11_MAP_WRITE_DISCARD,//MapType
		NULL,//MAPFlags
		&mappedSubresource//pMappedResource
	);
	memcpy(
		mappedSubresource.pData,//dest
		sphere_normals,//source
		sizeof(sphere_normals)//size
	);
	gpID3D11DeviceContext->Unmap(
		gpID3D11Buffer_Sphere_VertexBuffer_Normals,//pResource
		NULL//subresource
	);

	//Indices
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = gNumElements * sizeof(short);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_IndexBuffer //ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer indexbuffer for Sphere vertices .\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer indexbuffer for Sphere vertices .\n");
	}


	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(
		gpID3D11Buffer_IndexBuffer,//pResource
		NULL,//SubResource
		D3D11_MAP_WRITE_DISCARD,//MapType
		NULL,//MAPFlags
		&mappedSubresource//pMappedResource
	);
	memcpy(
		mappedSubresource.pData,//dest
		sphere_elements,//source
		gNumElements * sizeof(short)//size
	);
	gpID3D11DeviceContext->Unmap(
		gpID3D11Buffer_IndexBuffer,//pResource
		NULL//subresource
	);


	//********************************Sphere DATA BINDING***********************************
}

HRESULT createAndSetConstantBuffers(void)
{
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
	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstanctBuffer);

	gpID3D11DeviceContext->PSSetConstantBuffers(
		0,//StartSlot
		1,//NUmber of Buffer
		&gpID3D11Buffer_ConstanctBuffer//ppConstantBuffer
	);
	return hr;
}
HRESULT createAndSetInputLayout(void)
{
	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2]; //for in attributes
	ZeroMemory(inputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC));
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;


	//input layout is created in init
	HRESULT hr = gpID3D11Device->CreateInputLayout(
		inputElementDesc,//pInputElementDesc
		2,//NumElements position and normal
		pID3DBlob_vertexShaderCode->GetBufferPointer(),//pShaderByteCodeWithInputSignature
		pID3DBlob_vertexShaderCode->GetBufferSize(),//ByteCodeLength
		&gpID3D11InputLayout//ppInputLayout
	);;
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
	pID3DBlob_vertexShaderCode->Release();
	pID3DBlob_vertexShaderCode = NULL;
	return hr;
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
	initVertexShader();
	initPixelShader();

	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//clear depth stencile view
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencileView, D3D11_CLEAR_DEPTH, 1.0, 0);

	//local variables
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixIdentity();
	XMMATRIX wvMatrix = XMMatrixIdentity();
	CBUFFER constantBuffer;

	gpID3D11DeviceContext->IASetVertexBuffers(
		0,//StartSlot VDG_Attribute_position
		1,//NUmBuffer
		&gpID3D11Buffer_Sphere_VertexBuffer_Position,//ppvertexBuffer
		&stride,//pstride
		&offset//pOffset
	);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(
		1,//StartSlot VDG_Attribute_color
		1,//NUmBuffer
		&gpID3D11Buffer_Sphere_VertexBuffer_Normals,//ppvertexBuffer
		&stride,//pstride
		&offset//pOffset
	);


	//select which vertex buffer to display
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, 
		DXGI_FORMAT_R16_UINT, 0); //R16 maps with short
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//translation is concerned with world matrix transformation
	
	/***************Draw Cube Starts*******************/
	//draw cube
	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	//scaleMatrix = XMMatrixScaling(0.75f, 0.75f, 0.75f);
	//rotationMatrix = rotationMatrix *XMMatrixRotationX(XMConvertToRadians(gfSquareAngleRotation));
	//rotationMatrix = rotationMatrix * XMMatrixRotationY(XMConvertToRadians(gfSquareAngleRotation));
	//rotationMatrix = rotationMatrix * XMMatrixRotationZ(XMConvertToRadians(gfSquareAngleRotation));
	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	
	//load the data into the constat buffer
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;

	if (gLightToggle == 1) {
		constantBuffer.lightToggle = 1;
		constantBuffer.la0 = XMVectorSet(light0_ambient[0], light0_ambient[1], light0_ambient[2], light0_ambient[3]);
		constantBuffer.ld0 = XMVectorSet(light0_diffuse[0], light0_diffuse[1], light0_diffuse[2], light0_diffuse[3]);
		constantBuffer.ls0 = XMVectorSet(light0_specular[0], light0_specular[1], light0_specular[2], light0_specular[3]);

		constantBuffer.la1 = XMVectorSet(light1_ambient[0], light1_ambient[1], light1_ambient[2], light1_ambient[3]);
		constantBuffer.ld1 = XMVectorSet(light1_diffuse[0], light1_diffuse[1], light1_diffuse[2], light1_diffuse[3]);
		constantBuffer.ls1 = XMVectorSet(light1_specular[0], light1_specular[1], light1_specular[2], light1_specular[3]);

		constantBuffer.la2 = XMVectorSet(light2_ambient[0], light2_ambient[1], light2_ambient[2], light2_ambient[3]);
		constantBuffer.ld2 = XMVectorSet(light2_diffuse[0], light2_diffuse[1], light2_diffuse[2], light2_diffuse[3]);
		constantBuffer.ls2 = XMVectorSet(light2_specular[0], light2_specular[1], light2_specular[2], light2_specular[3]);

		constantBuffer.ka = XMVectorSet(material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3]);
		constantBuffer.kd = XMVectorSet(material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3]);
		constantBuffer.ks = XMVectorSet(material_specular[0], material_specular[1], material_specular[2], material_specular[3]);

		constantBuffer.lightPosition0 = XMVectorSet(light0_position[0], light0_position[1], light0_position[2], light0_position[3]);
		constantBuffer.lightPosition1 = XMVectorSet(light1_position[0], light1_position[1], light1_position[2], light1_position[3]);
		constantBuffer.lightPosition2 = XMVectorSet(light2_position[0], light2_position[1], light2_position[2], light2_position[3]);

		constantBuffer.materialShininess = material_shininess;
	}

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstanctBuffer, 0, NULL, &constantBuffer, 0, 0);
	
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

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
	
	if (gpID3D11Buffer_Sphere_VertexBuffer_Position)
	{
		gpID3D11Buffer_Sphere_VertexBuffer_Position->Release();
		gpID3D11Buffer_Sphere_VertexBuffer_Position = NULL;
	}
	if (gpID3D11Buffer_Sphere_VertexBuffer_Normals)
	{
		gpID3D11Buffer_Sphere_VertexBuffer_Normals->Release();
		gpID3D11Buffer_Sphere_VertexBuffer_Normals = NULL;
	}
	if (gpID3D11Buffer_IndexBuffer)
	{
		gpID3D11Buffer_IndexBuffer->Release();
		gpID3D11Buffer_IndexBuffer = NULL;
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
	angleRotate = angleRotate + 0.1f;
	if (angleRotate >= 360.0f)
		angleRotate = angleRotate - 360.0f;

	light0_position[0] = 0.0f;
	light0_position[1] = 100.0f * ((float)cos(angleRotate) / 2.0f - (float)sin(angleRotate) / 2.0f);
	light0_position[2] = 100.0f * ((float)cos(angleRotate) / 2.0f + (float)sin(angleRotate) / 2.0f);
	light0_position[3] = 1.0f;

	light1_position[0] = 100.0f * ((float)cos(angleRotate) / 2.0f + (float)sin(angleRotate) / 2.0f);
	light1_position[1] = 0.0f;
	light1_position[2] = 100.0f * ((float)cos(angleRotate) / 2.0f - (float)sin(angleRotate) / 2.0f);
	light1_position[3] = 1.0f;



	light2_position[0] = 100.0f * ((float)cos(angleRotate) / 2.0f + (float)sin(angleRotate) / 2.0f);
	light2_position[1] = 100.0f * ((float)cos(angleRotate) / 2.0f - (float)sin(angleRotate) / 2.0f);
	light2_position[2] = 0.0f;
	light2_position[3] = 1.0f;
}

void log(const char* logMessage) {
	fopen_s(&gpFile, gszLogFileName, "a+");
	fprintf_s(gpFile, logMessage);
	fclose(gpFile);
}