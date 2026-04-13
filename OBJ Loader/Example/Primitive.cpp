#include "Primitive.h"
#include <DirectXColors.h>
#include <VertexTypes.h>
#include <cassert>

using DirectX::VertexPositionColor;

namespace
{
    const int NUM_VERTS = 3;
    VertexPositionColor primitiveVertices[NUM_VERTS] =
    {
        { Vector3(0.0f, 1.0f, 0.0f), Colors::Red.v },
        { Vector3(1.0f, -1.0f, 0.0f), Colors::Green.v },
        { Vector3(-1.0f, -1.0f, 0.0f), Colors::Blue.v }
    };
}

Primitive::Primitive()
{
    pVertexBuffer = nullptr;
    pEffect = nullptr;
    pInputLayout = nullptr;
}

Primitive::~Primitive()
{
    delete pEffect;
    if (pVertexBuffer != nullptr)
    {
        pVertexBuffer->Release();
        pVertexBuffer = nullptr;
    }

    if (pInputLayout != nullptr)
    {
        pInputLayout->Release();
        pInputLayout = nullptr;
    }
}

// ------------------------------------------------------------------------------------
// Initialize the vertex buffer
// ------------------------------------------------------------------------------------
void Primitive::InitializeGeometry(ID3D11Device* pDevice)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = NUM_VERTS * sizeof(VertexPositionColor);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = primitiveVertices;

    HRESULT hr = pDevice->CreateBuffer(&desc, &data, &pVertexBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"FAILED TO CREATE VERTEX BUFFER");
        assert(false);
    }
}

// ------------------------------------------------------------------------------------
// Initialzes the shaders
// ------------------------------------------------------------------------------------
void Primitive::InitializeShaders(ID3D11Device* pDevice)
{
    pEffect = new BasicEffect(pDevice);
    pEffect->SetLightingEnabled(false);
    pEffect->SetTextureEnabled(false);
    pEffect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;
    pEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    HRESULT hr = pDevice->CreateInputLayout(
        layoutDesc, 2,
        shaderByteCode, byteCodeLength,
        &pInputLayout
    );

    if (FAILED(hr))
    {
        OutputDebugString(L"Could not create input layout");
        assert(false);
    }
}

// ------------------------------------------------------------------------------------
// Draw the primitive
// ------------------------------------------------------------------------------------
void Primitive::Draw(ID3D11DeviceContext* pDeviceContext, const Matrix& world, const Matrix& view, const Matrix& projection)
{
    pEffect->SetWorld(world);
    pEffect->SetView(view);
    pEffect->SetProjection(projection);
    pEffect->Apply(pDeviceContext);

    pDeviceContext->IASetInputLayout(pInputLayout);
    pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    pDeviceContext->Draw(NUM_VERTS, 0);
}