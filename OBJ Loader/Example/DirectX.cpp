#include <string>
#include <sstream>
#include "DirectX.h"

using namespace std;



//----------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static DirectXClass *application = NULL;

	switch( msg )
	{
	case WM_CREATE:
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam; 		// Get a pointer to the window CREATESTRUCT structure by casting lParam.  Yields the parameters used to create the window. 
		application = (DirectXClass*) cs->lpCreateParams;      // Grab the lpCreateParam we set to the DirectXClass object pointer (this) on the CreateWindow call and cast it back to a pointer to a DirectXClass object 
		return 0;
	}

	if( application )  
		return application->ProcessWindowMessages(msg, wParam, lParam);
	else
		return DefWindowProc(hWnd, msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------------------------
LRESULT DirectXClass::ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_SIZE:   // User resizing the window
		if( D3DDevice )
		{
			if( !ResizeDirectXComponents(LOWORD(lParam), HIWORD(lParam)) )
				PostQuitMessage(0);

			return 0;
		}
		break;

	case WM_DESTROY:    // Window is being destroyed
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(mainWnd, msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::ResizeDirectXComponents(int width, int height)
{
	clientWidth = width;
	clientHeight = height;

	SAFE_RELEASE(BackBuffer);
	SAFE_RELEASE(RenderTargetView);    // release COM objects which are reconstructed
	SAFE_RELEASE(DepthStencilView);
	SAFE_RELEASE(DepthStencilBuffer);

	if( FAILED(SwapChain->ResizeBuffers(1, clientWidth, clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0) ))  // Resize the swap chain and recreate the render target view.
		ErrorMessage(L"Failed to resize swapchain");

	if( !InitializeRenderTarget() )   // Re-initialize the Render target based on new size
		return false;

	if( depthStencilUsed ) 
		CreateDepthStencilBuffer();

	InitializeViewPorts();    // Reset View Ports based on new size

	return true;
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::SetRasterState(D3D11_RASTERIZER_DESC rasterizerState)
{
	if( RasterState )
		RasterState->Release();

	if( FAILED(D3DDevice->CreateRasterizerState( &rasterizerState, &RasterState) ))
		return ErrorMessage(L"Raster State not created");

	DeviceContext->RSSetState(RasterState);
	return true;
} 

//----------------------------------------------------------------------------------------------------------------
void DirectXClass::RenderScene(void)
{

	timer.CheckTime();      //checking the time
	Update( (float) timer.GetTimeDeltaTime() );

	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
	DeviceContext->ClearRenderTargetView(RenderTargetView, (const float*) ClearColor);    // clear the back buffer with the indicated colour
	if( depthStencilUsed )
	{
		DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
		DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	Render();

	if( displayFPS )
		DisplayFramesPerSecond(5, 5);
		
	SwapChain->Present(PresentInterval, 0);   // Swap buffers
}

//----------------------------------------------------------------------------------------------
int DirectXClass::MessageLoop()
{
	MSG msg = {0};

	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				break;
		}
		RenderScene();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::InitializeDirect3D(void)
{
	if( !InitializeSwapChain() )
		return false;

	if( !InitializeRenderTarget() )
		return false;

	if( depthStencilUsed )
	{ 
		if( !CreateDepthStencilBuffer() )
			return false;
	}

	InitializeViewPorts();
	font.InitializeFont(D3DDevice, DeviceContext, L"..\\Font\\Arial16.spritefont");

	return true;
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::InitializeSwapChain(void)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;   // structure to hold the swap chain

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));         // clear out the struct for use
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	swapChainDesc.SampleDesc.Count = 1;                               // set the level of multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;                             // set the quality of multi-sampling
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // tell how the chain is to be used
	swapChainDesc.BufferCount = 1;                                    // create two buffers, one for the front, one for the back
	swapChainDesc.OutputWindow = this->mainWnd;                       // set the window to be used by Direct3D
	swapChainDesc.Windowed = TRUE;                                    // set to windowed or full-screen mode

	// create a D3D11 device class and swap chain class using the information in the swapChainDesc structure
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &D3DDevice, NULL, &DeviceContext );
	if ( FAILED( hr ) )
	{
		return ErrorMessage( L"Could not create swap chain");
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::InitializeRenderTarget(void)
{
	// setup the backbuffer as the rendering target
	this->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);    // get the address of the back buffer and store it in pBackBuffer
	if( FAILED(D3DDevice->CreateRenderTargetView(BackBuffer, NULL, &this->RenderTargetView) ) )      // create a rendering target using the backbuffer
		return ErrorMessage(L"Could not create Render Target");


	return true;
}

//----------------------------------------------------------------------------------------------------------------
bool DirectXClass::CreateDepthStencilBuffer(void)
{
	D3D11_TEXTURE2D_DESC descDepth;   // Depth stencil texture description variable

	descDepth.Width = clientWidth;    // Complete depth stencil texture description
	descDepth.Height = clientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;   

	HRESULT res = D3DDevice->CreateTexture2D(&descDepth, NULL, &DepthStencilBuffer);
	if( FAILED( res ))    // Create the depth stencil texture
		return ErrorMessage(L"Depth Stencil texture not created");

	res = D3DDevice->CreateDepthStencilView(DepthStencilBuffer, NULL, &DepthStencilView);
	if( FAILED(res ))  // Create the depth stencil view
		return ErrorMessage(L"Depth Stencil view not created");


	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	desc.FrontFace = defaultStencilOp;
	desc.BackFace = defaultStencilOp;

	res = D3DDevice->CreateDepthStencilState( &desc, &DepthStencilState );
	if (FAILED(res))  // Create the depth stencil view
		return ErrorMessage(L"Depth Stencil State Not Created");


	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
	return true;
}

//----------------------------------------------------------------------------------------------
void DirectXClass::InitializeViewPorts(void)
{
	D3D11_VIEWPORT viewport;              // struture to hold the viewport information

	// Create the Viewport 
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));    // clear out the struct for use
	viewport.TopLeftX = 0;                            // set the top left to 0
	viewport.TopLeftY = 0;                            // set the top right to 0
	viewport.Width = (FLOAT) clientWidth;               // set the width to the window's  
	viewport.Height = (FLOAT) clientHeight;             // set the height to the window's 
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	DeviceContext->RSSetViewports(1, &viewport);          // set the viewport
}

//----------------------------------------------------------------------------------------------
DirectXClass::DirectXClass(HINSTANCE hInstance)
{
	// Win32 variables
	instance = hInstance;
	mainWnd = 0;
	clientWidth = 1024;
	clientHeight = 768;

	// Direct 3D variables
	D3DDevice = NULL;                      
	SwapChain = NULL;                    
	RenderTargetView = NULL;     
	DepthStencilView = NULL;
	DepthStencilBuffer = NULL;
	RasterState = NULL;

	depthStencilUsed = true;
	displayFPS = true;
	
	ClearColor = DirectX::SimpleMath::Color( 0.0f, 0.0f, 0.3f, 1.0f );

	PresentInterval = 1;

	timer.CheckTime();
}

//----------------------------------------------------------------------------------------------------------------
void DirectXClass::DisplayFramesPerSecond(int xPos, int yPos)
{
	wostringstream message;   // new string types required for output
	wstring messageOut;

	message << L"FPS....." << timer.GetFramesPerSecond();
	messageOut = message.str();

	font.PrintMessage(xPos, yPos, messageOut.c_str(), FC_GREEN);
}

//----------------------------------------------------------------------------------------------
DirectXClass::~DirectXClass()
{
	SAFE_RELEASE( DepthStencilState )
	SAFE_RELEASE( RenderTargetView );
	SAFE_RELEASE( BackBuffer );
	SAFE_RELEASE( SwapChain );
	SAFE_RELEASE( DeviceContext );
	SAFE_RELEASE( D3DDevice );
	SAFE_RELEASE( RasterState );
	SAFE_RELEASE( DepthStencilView );
}


//----------------------------------------------------------------------------------------------
//  Win32 Window Initialization Methods
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
bool DirectXClass::ErrorMessage(const LPCWSTR errorMsg)
{
	MessageBox(mainWnd, errorMsg, L"Fatal Error!", MB_ICONERROR);
	return false;  
}

//----------------------------------------------------------------------------------------------
bool DirectXClass::InitWindowsApp(wstring windowTitle, int nShowCmd)
{
	WNDCLASS wc; // Describe the window characteristics we want by filling out a WNDCLASS structure.
	int windowWidth, windowHeight;
	RECT rect = { 0, 0, clientWidth, clientHeight };		// Used to adjust the window size so the client area is required size

	// Determine the window rectangle dimensions based on requested client area dimensions.
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	windowWidth  = rect.right - rect.left;			// windowWidth is set to the client width requested plus the window border size
	windowHeight = rect.bottom - rect.top;			// windowHeight is set to the client height requested plus the window border size

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = this->instance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"BasicWndClass";

	if(!RegisterClass(&wc)) // Register the WNDCLASS instance with Windows so that we can create a window based on it.
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);
		return false;
	}

	// Create the Window based on the WNDClass we defined above
	mainWnd = CreateWindow(
		L"BasicWndClass",    // Registered WNDCLASS instance to use.
		windowTitle.c_str(), // window title
		WS_OVERLAPPEDWINDOW, // style flags
		CW_USEDEFAULT,       // x-coordinate
		CW_USEDEFAULT,       // y-coordinate
		windowWidth,	 	 // width
		windowHeight,	 	 // height
		0,                   // parent window
		0,                   // menu handle
		this->instance,      // app instance
		this);               // extra creation parameters, catch it in the winProc WM_Create and hook up 

	if(mainWnd == 0)
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		return false;
	}

	ShowWindow(mainWnd, nShowCmd);
	UpdateWindow(mainWnd);

	return true;
}
