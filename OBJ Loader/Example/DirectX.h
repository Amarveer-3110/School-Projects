#include <windows.h>
#include <d3d11_1.h>
#include <simplemath.h>	// for colour

#include "Timer.h"
#include "Font.h"

using namespace std;

#ifndef H_DirectX
#define H_DirectX

// Safe release macro, for releasing directX objects
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) if ( x != NULL ) { x->Release(); x = NULL; } 
#endif


class DirectXClass
{
  private:
    bool depthStencilUsed;
    bool displayFPS;
 
  protected:
    // Win32 variables
    HINSTANCE instance;     // Window instance
    HWND mainWnd;           // main window handle
    int clientWidth;        // window client width
    int clientHeight;       // window client height

    // Direct 3D variables
    ID3D11Device *D3DDevice;                      // Direct3D Device interface pointer
	ID3D11DeviceContext* DeviceContext;			  // Direct3D 11 device context 
    IDXGISwapChain *SwapChain;                    // Swap chain class pointer
    ID3D11RenderTargetView *RenderTargetView;     // Render Target view pointer
    ID3D11DepthStencilView *DepthStencilView;
    ID3D11Texture2D *DepthStencilBuffer;
	ID3D11Texture2D *BackBuffer;
    ID3D11RasterizerState *RasterState;

	ID3D11DepthStencilState* DepthStencilState;

    TimerType timer;
    FontType  font;
	int PresentInterval;							// controls VSync locking
	
	DirectX::SimpleMath::Color	ClearColor;
	

    bool InitializeSwapChain(void);
    bool InitializeRenderTarget(void);
    void InitializeViewPorts(void);
    void RenderScene(void);

	// override these in the base class to do the appropriate work
	virtual void Render() {};
	virtual void Update(float deltaTime) {};

  public:

    // Win32 methods ---------------------------------------------------------------
	bool InitWindowsApp(wstring windowTitle, int nShowCmd);	// Initialize the window class, register it and create the window.  Use default window dimensions.
    bool ErrorMessage(const LPCWSTR errorMsg);              // Generic error message
    int MessageLoop();                                      // Application mesaage loop

    void SetHandleInstance(HINSTANCE hInstance) { instance = hInstance; }
    void SetClientWidth(int width) { clientWidth = width; }
    void SetClientHeight(int height) { clientHeight = height; }
    HWND GetMainWnd() { return mainWnd; }                   // Accessor method for the window handle


    // Direct 3D methods ------------------------------------------------------------
    bool InitializeDirect3D(void);
    virtual LRESULT ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam);
    bool ResizeDirectXComponents(int width, int height);
    bool SetRasterState(D3D11_RASTERIZER_DESC rasterizerState);
    bool CreateDepthStencilBuffer(void);

    void SetDepthStencil(bool value) { depthStencilUsed = value; }
    ID3D11Device* GetD3DDevice() { return D3DDevice; }


    void DisplayFramesPerSecond(int xPos, int yPos);
    bool DisplayFPS( bool value) { displayFPS = value; }

    DirectXClass(HINSTANCE hInstance);     // Default Constructor
    DirectXClass();                        // Constructor with the instance variable passed through by WinMain
    virtual ~DirectXClass();                       // Destructor
};

#endif