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
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Color = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstanctBuffer = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX gPerspectiveProjectionMatrix;

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);

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

	float vertices_position[] =
	{
		0.0f, 1.0f, 0.0f, //apex
		1.0f, -1.0f, 0.0f, //right
		-1.0f, -1.0f, 0.0f, //left
	};

	float vertices_color[] =
	{
		1.0f, 0.0f, 0.0f, //appex
		0.0f, 0.0f, 1.0f, //right
		0.0f, 1.0f, 0.0f //left
	};

	//create vertex position buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(vertices_position);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL,
		&gpID3D11Buffer_VertexBuffer_Position);

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
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Position, NULL, D3D11_MAP_WRITE_DISCARD,
		NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, vertices_position, sizeof(vertices_position));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Position, NULL);

	//create vertex color buffer
	D3D11_BUFFER_DESC bufferDescCol;
	ZeroMemory(&bufferDescCol, sizeof(D3D11_BUFFER_DESC));
	bufferDescCol.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescCol.ByteWidth = sizeof(float) * ARRAYSIZE(vertices_color);
	bufferDescCol.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescCol.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescCol, NULL, &gpID3D11Buffer_VertexBuffer_Color);
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
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Color, NULL, 
		D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResourceForColor);
	memcpy(mappedSubResourceForColor.pData, vertices_color, sizeof(vertices_color));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Color, NULL);


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
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 1.0f;

	//set projection matrix to identity matrix
	gPerspectiveProjectionMatrix = XMMatrixIdentity();

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
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	//select which vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;

	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Color, &stride, &offset);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation is concerned with world matrix transformation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX modelMatrix = XMMatrixIdentity();

	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	//final world view projection matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	//load the data into the constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstanctBuffer, 0, NULL, &constantBuffer, 0,0);

	//draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(3, 0);


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
	if (gpID3D11Buffer_VertexBuffer_Position)
	{
		gpID3D11Buffer_VertexBuffer_Position->Release();
		gpID3D11Buffer_VertexBuffer_Position = NULL;
	}
	if (gpID3D11Buffer_VertexBuffer_Color)
	{
		gpID3D11Buffer_VertexBuffer_Color->Release();
		gpID3D11Buffer_VertexBuffer_Color = NULL;
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

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
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