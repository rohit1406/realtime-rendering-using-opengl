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
void log(char logMessage[]);
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
//for tesselation
ID3D11HullShader *gpID3D11HullShader = NULL;
ID3D11DomainShader *gpID3D11DomainShader = NULL;
ID3D11Buffer *gpID3D11Buffer_vertexBuffer_position = NULL; //vbo position
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_constantBuffer_hull = NULL;
ID3D11Buffer *gpID3D11Buffer_constantBuffer_domain = NULL;
ID3D11Buffer *gpID3D11Buffer_constantBuffer_pixel = NULL;

//cbuffer size must be in multiple of >=16bit for padding between variables in shaders
struct CBUFFER_HULLSHADER {
	XMVECTOR Hull_Constant_Function_Param; //0th member NumberOfStrips and 1st member numberOfLineSegment
};

struct CBUFFER_DOMAINSHADER {
	XMMATRIX WorldViewProjectionMatrix;
};

struct CBUFFER_PIXELSHADER {
	XMVECTOR LineColor;
};

unsigned int gNumberOfLineSegments = 1;

ID3D11RasterizerState *gpRS;

//depth related changes
ID3D11DepthStencilView *gpID3D11DepthStencileView = NULL;

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
			//updateAngle();
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

		case VK_UP:
			gNumberOfLineSegments++;
			if (gNumberOfLineSegments >= 30)
				gNumberOfLineSegments = 30;
			break;

		case VK_DOWN:
			gNumberOfLineSegments--;
			if (gNumberOfLineSegments <= 0)
				gNumberOfLineSegments = 1;
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

void log(char logMessage[]) {
	fopen_s(&gpFile, gszLogFileName, "a+");
	fprintf_s(gpFile, logMessage);
	fclose(gpFile);
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
		"struct VertexOut {"\
		"float4 position: POSITION;"\
		"};"\

		"struct VertexIN{"\
		"float2 pos: POSITION;"\
		"};"\

		"VertexOut main(VertexIN vin)"\
		"{"\
		"VertexOut output;"\
		"output.position = float4(vin.pos,1.0f,1.0f);"\
		"return output;"\
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

	//**************HULL Shader // tesselation control opengl***************/
	const char* hullShaderSourceCode =
		"cbuffer ConstantBuffer {"\
		"float4 hull_constant_function_param;"\
		"}"\

		"struct VertexOut {"\
		"float4 position: POSITION;"\
		"};"\

		"struct hull_constant_output {"\
		"float edges[2]: SV_TessFactor; "\
		"};"\

		"hull_constant_output hull_constant_function(void) {"\
		"hull_constant_output output;"\
		"float numberOfStrip = hull_constant_function_param[0]; "\
		"float numberOfLineSegments = hull_constant_function_param[1]; "\
		"output.edges[0] = numberOfStrip;"\
		"output.edges[1] = numberOfLineSegments;"\
		"return output; "\
		"}"\

		"struct hull_output {"\
		"float4 position: POSITION;"\
		"};"\

		"[domain(\"isoline\")]"\
		"[partitioning(\"integer\")]"\
		"[outputtopology(\"line\")]"\
		"[outputcontrolpoints(4)]"\
		"[patchconstantfunc(\"hull_constant_function\")]"\
		"hull_output main(InputPatch<VertexOut,4> inputPatch,uint i: SV_OutputControlPointID)"\
		"{"\
		"hull_output output;"\
		"output.position = inputPatch[i].position;"\
		"return output;"\
		"}";

	ID3DBlob *pID3DBlob_HullShaderCode = NULL;
	ID3DBlob *pID3DBlob_HullError = NULL;

	hr = D3DCompile(
		hullShaderSourceCode,//psrcData
		lstrlenA(hullShaderSourceCode) + 1,//size
		"HS",//pSourceName
		NULL,//d3d_shader_macro if any
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
		"main",//entry point
		"hs_5_0",//pTarget
		0,//flags1
		0,//flags2
		&pID3DBlob_HullShaderCode,//ppCode
		&pID3DBlob_HullError//perrormessage
	);

	if (FAILED(hr))
	{
		if (pID3DBlob_HullError != NULL)
		{
			log("Failed: D3DCompile for Hull shader : ");
			log((char*)pID3DBlob_HullError->GetBufferPointer());
			log("\n\n");
			pID3DBlob_HullError->Release();
			pID3DBlob_HullError = NULL;
			return hr;
		}
	}
	else {
		log("Success: D3DCompile for Hull shader.\n");
	}

	hr = gpID3D11Device->CreateHullShader(
		pID3DBlob_HullShaderCode->GetBufferPointer(),//pShaderByteCode
		pID3DBlob_HullShaderCode->GetBufferSize(), //bytecode length
		NULL,//pClassLinkage (if u have created uniform which is common accross shaders)
		&gpID3D11HullShader //ppVertexShader
	);

	if (FAILED(hr))
		log("Failed: gpID3D11Device->CreateHullShader.\n");
	else
		log("Success: gpID3D11Device->CreateHullShader.\n");

	gpID3D11DeviceContext->HSSetShader(
		gpID3D11HullShader,//pVertexShader
		0,//ppClassInstances
		0//numClassInstances
	);
	//**************HULL Shader END***************/

	//**************Domain Shader // tesselation evaluation opengl***************/
	const char* domainShaderSourceCode =
		"cbuffer ConstantBuffer {"\
		"float4x4 WorldViewProjectionMatrix;"\
		"}"\

		"struct hull_output {"\
		"float4 position: POSITION;"\
		"};"\

		"struct domain_output {"\
		"float4 position: SV_POSITION;"\
		"};"\

		"struct hull_constant_output {"\
		"float edges[2] : SV_TessFactor; "\
		"};"\


		"[domain(\"isoline\")]"\
		"domain_output main(hull_constant_output inputEdges,OutputPatch<hull_output,4> outputPatch,float2 tessCoordUV: SV_DomainLocation)"\
		"{"\
		"domain_output output;"\
		" float u = tessCoordUV.x;				"\
		" float3 p0 = outputPatch[0].position.xyz;	"\
		" float3 p1 = outputPatch[1].position.xyz;	"\
		" float3 p2 = outputPatch[2].position.xyz;	"\
		" float3 p3 = outputPatch[3].position.xyz;	"\
		" float u1 = (1.0 - u);				"\
		" float u2 = (u * u);				"\
		" float b3 = (u2 *  u);				"\
		" float b2 = (3.0 * u2 * u1);		"\
		" float b1 = (3.0 * u * u1 * u1);	"\
		" float b0 = (u1 * u1 * u1);		"\
		" float3 p  = p0 * b0  + p1 * b1 + p2 * b2 + p3 * b3; "\
		"output.position = mul(WorldViewProjectionMatrix , float4(p,1.0f));"\
		"return output;"\
		"}";

	ID3DBlob *pID3DBlob_DomainShaderCode = NULL;
	ID3DBlob *pID3DBlob_DomainError = NULL;

	hr = D3DCompile(
		domainShaderSourceCode,//psrcData
		lstrlenA(domainShaderSourceCode) + 1,//size
		"DS",//pSourceName
		NULL,//d3d_shader_macro if any
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//pInclude
		"main",//entry point
		"ds_5_0",//pTarget
		0,//flags1
		0,//flags2
		&pID3DBlob_DomainShaderCode,//ppCode
		&pID3DBlob_DomainError//perrormessage
	);

	if (FAILED(hr))
	{
		if (pID3DBlob_DomainError != NULL)
		{
			log("Failed: D3DCompile for domain shader : ");
			log((char*)pID3DBlob_DomainError->GetBufferPointer());
			log("\n\n");
			pID3DBlob_DomainError->Release();
			pID3DBlob_DomainError = NULL;
			return hr;
		}
	}
	else {
		log("Success: D3DCompile for domain shader.\n");
	}

	hr = gpID3D11Device->CreateDomainShader(
		pID3DBlob_DomainShaderCode->GetBufferPointer(),//pShaderByteCode
		pID3DBlob_DomainShaderCode->GetBufferSize(), //bytecode length
		NULL,//pClassLinkage (if u have created uniform which is common accross shaders)
		&gpID3D11DomainShader //ppVertexShader
	);

	if (FAILED(hr))
		log("Failed: gpID3D11Device->CreateDomainShader.\n");
	else
		log("Success: gpID3D11Device->CreateDomainShader.\n");

	gpID3D11DeviceContext->DSSetShader(
		gpID3D11DomainShader,//pVertexShader
		0,//ppClassInstances
		0//numClassInstances
	);
	//**************HULL Shader END***************/



	//float4 position: SV_POSITION,float4 color: COLOR
	const char* pixelShaderSourceCode =
		"cbuffer ConstantBuffer {"\
		"float4 lineColor;"\
		"}"\
		"struct VertexOut{"\
		"float4 pos: SV_POSITION;"\
		"float4 col: COLOR;"\
		"};"\

		"float4 main(VertexOut vIn) : SV_TARGET"\
		"{"\
		"return lineColor;"\
		"}";

	ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
	ID3DBlob *pID3DBlob_error = NULL;

	hr = D3DCompile(
		pixelShaderSourceCode,//psrcData
		lstrlenA(pixelShaderSourceCode) + 1,//size
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

	const float vertices[] = {
		-1.0f,-1.0f,
		-0.5f,1.0f,
		0.5,-1.0f,
		1.0f,1.0f
	};


	//create vertex buffer //similar to vbo in opengl
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_vertexBuffer_position//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for line vertices position.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for line vertices poaition.\n");
	}

	//now copy vertices into above created buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(
		gpID3D11Buffer_vertexBuffer_position,//pResource
		NULL,//SubResource
		D3D11_MAP_WRITE_DISCARD,//MapType
		NULL,//MAPFlags
		&mappedSubresource//pMappedResource
	);
	memcpy(
		mappedSubresource.pData,//dest
		vertices,//source
		sizeof(vertices)//size
	);
	gpID3D11DeviceContext->Unmap(
		gpID3D11Buffer_vertexBuffer_position,//pResource
		NULL//subresource
	);


	//Create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[1];//4={}
	ZeroMemory(inputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC));
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0; // like VDG_ATTRIBUTE_Position
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	gpID3D11Device->CreateInputLayout(
		inputElementDesc,//pInputElementDesc
		1,//NumElements position 
		pID3DBlob_VertexShaderCode->GetBufferPointer(),//pShaderByteCodeWithInputSignature
		pID3DBlob_VertexShaderCode->GetBufferSize(),//ByteCodeLength
		&gpID3D11InputLayout//ppInputLayout
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateInputLayout.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateInputLayout.\n");
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	//Create Constant buffer
	D3D11_BUFFER_DESC bufferDesc_constantBuffer;
	ZeroMemory(&bufferDesc_constantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_constantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_constantBuffer.ByteWidth = sizeof(CBUFFER_HULLSHADER);
	bufferDesc_constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc_constantBuffer,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_constantBuffer_hull//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for constant buffer hull.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for constant buffer hull.\n");
	}

	gpID3D11DeviceContext->HSSetConstantBuffers(
		0,//StartSlot
		1,//NUmber of Buffer
		&gpID3D11Buffer_constantBuffer_hull//ppConstantBuffer
	);

	ZeroMemory(&bufferDesc_constantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_constantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_constantBuffer.ByteWidth = sizeof(CBUFFER_DOMAINSHADER);
	bufferDesc_constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc_constantBuffer,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_constantBuffer_domain//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for constant buffer domain.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for constant buffer domain.\n");
	}

	gpID3D11DeviceContext->DSSetConstantBuffers(
		0,//StartSlot
		1,//NUmber of Buffer
		&gpID3D11Buffer_constantBuffer_domain//ppConstantBuffer
	);

	ZeroMemory(&bufferDesc_constantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_constantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_constantBuffer.ByteWidth = sizeof(CBUFFER_PIXELSHADER);
	bufferDesc_constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpID3D11Device->CreateBuffer(
		&bufferDesc_constantBuffer,//pDesc
		NULL,//subresourcedata pInitialData
		&gpID3D11Buffer_constantBuffer_pixel//ppBuffer
	);

	if (FAILED(hr)) {
		log("Failed: gpID3D11Device->CreateBuffer for constant buffer domain.\n");
	}
	else {
		log("Success: gpID3D11Device->CreateBuffer for constant buffer domain.\n");
	}

	gpID3D11DeviceContext->PSSetConstantBuffers(
		0,//StartSlot
		1,//NUmber of Buffer
		&gpID3D11Buffer_constantBuffer_pixel//ppConstantBuffer
	);

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
	UINT stride = sizeof(float) * 2;
	UINT offset = 0;

	//*****************************Line DRAW BEGIN**************************************
	gpID3D11DeviceContext->IASetVertexBuffers(
		0,//StartSlot VDG_Attribute_position
		1,//NUmBuffer
		&gpID3D11Buffer_vertexBuffer_position,//ppvertexBuffer
		&stride,//pstride
		&offset//pOffset
	);

	//select geometry premetive
	gpID3D11DeviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST //topology
	);

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f); // Z is + inword
	worldMatrix *= translationMatrix;

	//final w v p matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	//load data into const buffer
	CBUFFER_HULLSHADER constantBufferHullShader;
	constantBufferHullShader.Hull_Constant_Function_Param = XMVectorSet(1, gNumberOfLineSegments, 0, 0);

	TCHAR str[255];
	wsprintf(str, TEXT("DirextX11 Window : [ Segments = %d ]"), gNumberOfLineSegments);
	SetWindowText(ghwnd, str);


	gpID3D11DeviceContext->UpdateSubresource(
		gpID3D11Buffer_constantBuffer_hull,//PdstResource
		0,//dstSubresource
		NULL,//pDstBox
		&constantBufferHullShader,//pSrcData
		0,//srcRowPitch
		0//srcDepthPitch
	);

	CBUFFER_DOMAINSHADER constantBufferDomainShader;
	constantBufferDomainShader.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(
		gpID3D11Buffer_constantBuffer_domain,//PdstResource
		0,//dstSubresource
		NULL,//pDstBox
		&constantBufferDomainShader,//pSrcData
		0,//srcRowPitch
		0//srcDepthPitch
	);

	//load data into const buffer
	CBUFFER_PIXELSHADER constantBufferPixelShader;
	constantBufferPixelShader.LineColor = XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
	gpID3D11DeviceContext->UpdateSubresource(
		gpID3D11Buffer_constantBuffer_pixel,//PdstResource
		0,//dstSubresource
		NULL,//pDstBox
		&constantBufferPixelShader,//pSrcData
		0,//srcRowPitch
		0//srcDepthPitch
	);

	//draw
	/*gpID3D11DeviceContext->Draw(
		12,//vertex count
		0//start vertex location
	);*/
	//*****************************Line DRAW END**************************************


	//draw
	gpID3D11DeviceContext->Draw(
		4,//vertex count
		0//start vertex location
	);

	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);
}

void uninitialize(void)
{
	//code
	if (gpID3D11Buffer_constantBuffer_hull)
	{
		gpID3D11Buffer_constantBuffer_hull->Release();
		gpID3D11Buffer_constantBuffer_hull = NULL;
	}
	if (gpID3D11Buffer_constantBuffer_domain)
	{
		gpID3D11Buffer_constantBuffer_domain->Release();
		gpID3D11Buffer_constantBuffer_domain = NULL;
	}
	if (gpID3D11Buffer_constantBuffer_pixel)
	{
		gpID3D11Buffer_constantBuffer_pixel->Release();
		gpID3D11Buffer_constantBuffer_pixel = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}
	if (gpID3D11Buffer_vertexBuffer_position)
	{
		gpID3D11Buffer_vertexBuffer_position->Release();
		gpID3D11Buffer_vertexBuffer_position = NULL;
	}
	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11HullShader)
	{
		gpID3D11HullShader->Release();
		gpID3D11HullShader = NULL;
	}
	if (gpID3D11DomainShader)
	{
		gpID3D11DomainShader->Release();
		gpID3D11DomainShader = NULL;
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
