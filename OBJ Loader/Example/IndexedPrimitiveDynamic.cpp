#include "IndexedPrimitiveDynamic.h"
#include <cassert>

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------
IndexedPrimitiveDynamic::IndexedPrimitiveDynamic()
{
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
    inputLayout = nullptr;
    pEffect = nullptr;
    indexCount = 0;
}

IndexedPrimitiveDynamic::~IndexedPrimitiveDynamic()
{
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
    if (inputLayout) inputLayout->Release();
    delete pEffect;
}

void IndexedPrimitiveDynamic::InitializeShaders(ID3D11Device* pDevice)
{
    pEffect = new BasicEffect(pDevice);
    pEffect->SetLightingEnabled(false);
    pEffect->SetTextureEnabled(false);
    pEffect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;
    pEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    HRESULT hr = pDevice->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        shaderByteCode,
        byteCodeLength,
        &inputLayout
    );

    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create input layout (Dynamic)");
        assert(false);
    }
}

bool IndexedPrimitiveDynamic::InitializeGeometry(ID3D11Device* device, const std::vector<VertexPositionColor>& vertices, const std::vector<DWORD>& indices)
{
    if (vertexBuffer)
    {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }

    if (indexBuffer)
    {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }

    indexCount = (int)indices.size();

    if (vertices.empty() || indices.empty())
        return false;

    // ----------------------------
    // Create Vertex Buffer
    // ----------------------------
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)(vertices.size() * sizeof(VertexPositionColor));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&desc, &data, &vertexBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"FAILED TO CREATE DYNAMIC VERTEX BUFFER");
        assert(false);
        return false;
    }

    D3D11_BUFFER_DESC indexDesc = {};
    indexDesc.ByteWidth = (UINT)(indices.size() * sizeof(DWORD));
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"FAILED TO CREATE DYNAMIC INDEX BUFFER");
        assert(false);
        return false;
    }

    return true;
}

void IndexedPrimitiveDynamic::Draw(ID3D11DeviceContext* context, const Matrix& world, const Matrix& view, const Matrix& projection)
{
    if (!vertexBuffer || !indexBuffer || !pEffect || !inputLayout)
        return;

    pEffect->SetWorld(world);
    pEffect->SetView(view);
    pEffect->SetProjection(projection);
    pEffect->Apply(context);

    context->IASetInputLayout(inputLayout);
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(indexCount, 0, 0);
}