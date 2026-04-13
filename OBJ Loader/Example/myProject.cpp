#include "MyProject.h"
#include <Windowsx.h>
#include <d3d11_1.h>
#include <SimpleMath.h>
#include <DirectXColors.h>
#include <sstream>
#include <CommonStates.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

float RandFloat() { return float(rand()) / float(RAND_MAX); }
static const float CAMERA_SPEED = XM_PI * 0.2f;

//----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
    MyProject application(hInstance);

    if (application.InitWindowsApp(L"DirectX 3D", nShowCmd) == false)
    {
        return 0;
    }

    if (application.InitializeDirect3D())
    {
        application.SetDepthStencil(true);
        application.InitializeTextures();
        application.InitializeObjects();
        application.MessageLoop();
    }

    return 0;
}

//----------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------
MyProject::MyProject(HINSTANCE hInstance)
    : DirectXClass(hInstance)
{
    mousePos = Vector2(clientWidth * 0.5f, clientHeight * 0.5f);
    buttonDown = false;

    ClearColor = Color(DirectX::Colors::Black.v);

    cameraPos = Vector3(0, 0, 6);
    cameraRadius = 8;
    cameraRadiusSpeed = 0;
    cameraRotation = Vector2::Zero;
    cameraRotationSpeed = Vector2::Zero;
}

//----------------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------------
MyProject::~MyProject()
{
}

void MyProject::InitializeTextures()
{
    spriteBatch = new SpriteBatch(DeviceContext);

    uiTexture.Load(D3DDevice, L"..\\Textures\\block.png");
}

//----------------------------------------------------------------------------------------------
// Initialize any fonts we need to use here
//----------------------------------------------------------------------------------------------
void MyProject::InitializeObjects()
{
    worldMatrix = Matrix::Identity;
    parentMatrix = Matrix::Identity;

    displayRotation = 0.0f;
    wireframeMode = false;
    scaledUp = false;

    objLoader.Load("..\\Models\\sample.obj", vertices, indices);
    dynamicMesh.InitializeGeometry(D3DDevice, vertices, indices);
    dynamicMesh.InitializeShaders(D3DDevice);

    loadButton.Initialize(&uiTexture, Vector2(80, 40), 0.0f, 0.35f, Color(DirectX::Colors::White.v), 0.0f);
    colorButton.Initialize(&uiTexture, Vector2(80, 130), 0.0f, 0.35f, Color(DirectX::Colors::White.v), 0.0f);
    scaleButton.Initialize(&uiTexture, Vector2(80, 220), 0.0f, 0.35f, Color(DirectX::Colors::White.v), 0.0f);
}

//----------------------------------------------------------------------------------------------
// Window message handler
//----------------------------------------------------------------------------------------------
LRESULT MyProject::ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
    Vector2 pos((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));

    switch (msg)
    {
    case WM_MOUSEMOVE:
        mouseDelta = pos - mousePos;
        mousePos = pos;
        OnMouseMove();
        return 0;

    case WM_LBUTTONUP:
        buttonDown = false;
        mouseDelta = pos - mousePos;
        mousePos = pos;
        break;

    case WM_LBUTTONDOWN:
        buttonDown = true;
        mouseDelta = pos - mousePos;
        mousePos = pos;
        OnMouseDown();
        break;

    case WM_KEYUP:
        if (wParam >= '0' && wParam <= '4')
        {
            PresentInterval = wParam - '0';
        }
        else if (wParam == VK_UP) { cameraRotationSpeed.y = 0; }
        else if (wParam == VK_DOWN) { cameraRotationSpeed.y = 0; }
        else if (wParam == VK_LEFT) { cameraRotationSpeed.x = 0; }
        else if (wParam == VK_RIGHT) { cameraRotationSpeed.x = 0; }
        else if (wParam == VK_ADD) { cameraRadiusSpeed = 0; }
        else if (wParam == VK_SUBTRACT) { cameraRadiusSpeed = 0; }
        else if (wParam == VK_SPACE)
        {
            cameraRotation = Vector2::Zero;
            cameraRadius = 6;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_UP) { cameraRotationSpeed.y = CAMERA_SPEED; }
        else if (wParam == VK_DOWN) { cameraRotationSpeed.y = -CAMERA_SPEED; }
        else if (wParam == VK_LEFT) { cameraRotationSpeed.x = -CAMERA_SPEED; }
        else if (wParam == VK_RIGHT) { cameraRotationSpeed.x = CAMERA_SPEED; }
        else if (wParam == VK_ADD) { cameraRadiusSpeed = -1.0f; }
        else if (wParam == VK_SUBTRACT) { cameraRadiusSpeed = 1.0f; }
        break;
    }

    return DirectXClass::ProcessWindowMessages(msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------
// Called by the render loop to render a single frame
//----------------------------------------------------------------------------------------------
void MyProject::Render(void)
{
    ComputeViewProjection();

    worldMatrix = parentMatrix;
    if (scaledUp)
        worldMatrix *= Matrix::CreateScale(1.5f);

    dynamicMesh.Draw(DeviceContext, worldMatrix, viewMatrix, projectionMatrix);

    spriteBatch->Begin();

    loadButton.Draw(spriteBatch);
    colorButton.Draw(spriteBatch);
    scaleButton.Draw(spriteBatch);

    spriteBatch->End();

    DirectXClass::Render();
}

//----------------------------------------------------------------------------------------------
// Called every frame to update objects.
//	deltaTime: how much time in seconds has elapsed since the last frame
//----------------------------------------------------------------------------------------------
void MyProject::Update(float deltaTime)
{
    UpdateCamera(deltaTime);
    displayRotation += deltaTime * XM_PI / 10.0f;
    parentMatrix = Matrix::CreateRotationY(displayRotation);
}

//----------------------------------------------------------------------------------------------
// Called when the mouse is released
//----------------------------------------------------------------------------------------------
void MyProject::OnMouseDown()
{
    if (loadButton.ContainsPoint(mousePos))
    {
        vertices.clear();
        indices.clear();

        if (objLoader.LoadFromDialog(mainWnd, vertices, indices))
            dynamicMesh.InitializeGeometry(D3DDevice, vertices, indices);
    }
    else if (colorButton.ContainsPoint(mousePos))
    {
        Color meshColor(RandFloat(), RandFloat(), RandFloat(), 1.0f);

        for (auto& v : vertices)
        {
            v.color = meshColor;
        }

        if (!vertices.empty() && !indices.empty())
            dynamicMesh.InitializeGeometry(D3DDevice, vertices, indices);
    }
    else if (scaleButton.ContainsPoint(mousePos))
    {
        scaledUp = !scaledUp;
    }
}

//----------------------------------------------------------------------------------------------
// Called when the mouse is moved
//----------------------------------------------------------------------------------------------
void MyProject::OnMouseMove()
{
}

//----------------------------------------------------------------------------------------------
// Computes the view and camera matrix
//----------------------------------------------------------------------------------------------
void MyProject::ComputeViewProjection()
{
    viewMatrix = Matrix::CreateLookAt(cameraPos, Vector3::Zero, Vector3::UnitY);
    projectionMatrix = Matrix::CreatePerspectiveFieldOfView(60.0f * XM_PI / 180.0f, (float)clientWidth / (float)clientHeight, 1, 20);
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void MyProject::UpdateCamera(float deltaTime)
{
    const float VERT_LIMIT = XM_PI * 0.35f;

    cameraRadius += cameraRadiusSpeed * deltaTime;
    if (cameraRadius < 1) cameraRadius = 1;

    cameraRotation += cameraRotationSpeed * deltaTime;

    if (cameraRotation.y < -VERT_LIMIT) cameraRotation.y = -VERT_LIMIT;
    else if (cameraRotation.y > VERT_LIMIT) cameraRotation.y = VERT_LIMIT;

    cameraPos.y = cameraRadius * sinf(cameraRotation.y);
    float r = cameraRadius * cosf(cameraRotation.y);

    cameraPos.x = sinf(cameraRotation.x) * r;
    cameraPos.z = cosf(cameraRotation.x) * r;
}