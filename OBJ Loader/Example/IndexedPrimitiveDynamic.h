#pragma once
#include <d3d11_1.h>
#include <vector>
#include <SimpleMath.h>
#include <Effects.h>
#include <VertexTypes.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// import the DirectXTK input layout and structure
using DirectX::VertexPositionColor;

class IndexedPrimitiveDynamic
{
public:
    IndexedPrimitiveDynamic();   
    ~IndexedPrimitiveDynamic();  

    void InitializeShaders(ID3D11Device* device);

    bool InitializeGeometry(ID3D11Device* device, const std::vector<VertexPositionColor>& vertices, const std::vector<DWORD>& indices);

    void Draw(ID3D11DeviceContext* context, const Matrix& world, const Matrix& view, const Matrix& projection);

private:

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;

    ID3D11InputLayout* inputLayout;
    BasicEffect* pEffect;

    int indexCount;
};
