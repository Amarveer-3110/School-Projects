#ifndef _MyProject_h
#define _MyProject_h

#include "DirectX.h"
#include "Font.h"
#include "TextureType.h"
#include "Primitive.h"
#include "IndexedPrimitive.h"
#include "TransformLoader.h"
#include "OBJLoader.h"
#include "IndexedPrimitiveDynamic.h"
#include "Sprite.h"

// forward declare the sprite batch

using namespace DirectX;
using namespace DirectX::SimpleMath;


//----------------------------------------------------------------------------------------------
// Main project class
//	Inherits the directx class to help us initalize directX
//----------------------------------------------------------------------------------------------
class MyProject : public DirectXClass
{
public:
    MyProject(HINSTANCE hInstance);
    ~MyProject();

    void InitializeTextures();
    void InitializeObjects();
    LRESULT ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam);
    void Render(void);
    void Update(float deltaTime);
    void ComputeViewProjection();

private:
    OBJLoader objLoader;
    IndexedPrimitiveDynamic dynamicMesh;

    std::vector<VertexPositionColor> vertices;
    std::vector<DWORD> indices;

    Matrix parentMatrix;
    Matrix worldMatrix;
    Matrix viewMatrix;
    Matrix projectionMatrix;

    float displayRotation;

    SpriteBatch* spriteBatch;

    Vector2 mousePos;
    Vector2 mouseDelta;
    bool buttonDown;

    Vector3 cameraPos;
    Vector2 cameraRotationSpeed;
    Vector2 cameraRotation;
    float cameraRadius;
    float cameraRadiusSpeed;

    void UpdateCamera(float deltaTime);
    void OnMouseDown();
    void OnMouseMove();

    TextureType uiTexture;

    Sprite loadButton;
    Sprite colorButton;
    Sprite scaleButton;

    bool wireframeMode;
    bool scaledUp;
};

#endif