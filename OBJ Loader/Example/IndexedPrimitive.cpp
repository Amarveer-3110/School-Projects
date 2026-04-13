//
// BGTD 9201
//	Class that represents a simple IndexedPrimitive
//

#include "IndexedPrimitive.h"
#include <DirectXColors.h>
#include <VertexTypes.h>

// import the DirectXTK input layout and structure
using DirectX::VertexPositionColor;

//  Create an array of vertices
const int NUM_VERTS = 8;
VertexPositionColor vertices[NUM_VERTS] =
{
	// top vertices of cube  --------------------------------------------------------
	{ Vector3(1.0f, 1.0f, 1.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },  	// Vertex 0
	{ Vector3(1.0f, 1.0f, -1.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },	// Vertex 1
	{ Vector3(-1.0f, 1.0f, -1.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },     	// Vertex 2
	{ Vector3(-1.0f, 1.0f, 1.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },     	// Vertex 3    

	// bottom vertices of cube  --------------------------------------------------------
	{ Vector3(1.0f, -1.0f, 1.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },	// Vertex 4
	{ Vector3(1.0f, -1.0f, -1.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },   	 // Vertex 5             
	{ Vector3(-1.0f, -1.0f, -1.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },	// Vertex 6
	{ Vector3(-1.0f, -1.0f, 1.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },	// Vertex 7
};

// the indices
// DWORD is a 32 bit unsigned integer
const int NUM_FACES = 12;
const int NUM_INDICES = NUM_FACES * 3;
DWORD indices[NUM_INDICES] =
{
	0, 2, 1,	// top quad
	0, 3, 2,
	0, 1, 4,	// right quad
	1, 5, 4,
	1, 6, 5,	// front quad
	1, 2, 6,
	2, 7, 6,	// left quad
	2, 3, 7,
	3, 4, 7,	// back quad
	0, 4, 3,
	5, 6, 7,	// bottom quad
	5, 7, 4
};


// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
IndexedPrimitive::IndexedPrimitive()
{
	pVertexBuffer = nullptr;
	pEffect = nullptr;
	pInputLayout = nullptr;
	pIndexBuffer = nullptr;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
IndexedPrimitive::~IndexedPrimitive()
{
	// Make sure we clean up what ever we allocated!
	delete pEffect;
	if (pVertexBuffer != nullptr)
	{
		pVertexBuffer->Release();
		pVertexBuffer = nullptr;
	}
	if (pIndexBuffer != nullptr)
	{
		pIndexBuffer->Release();
		pIndexBuffer = nullptr;
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
void IndexedPrimitive::InitializeGeometry(ID3D11Device* pDevice)
{
	// describe the vertex buffer we are trying to create
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = NUM_VERTS * sizeof(VertexPositionColor);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// setup the subresource data - tells D3D what data to use to initialize
	// the vertexbuffer with
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices;

	// create the vertex buffer
	HRESULT hr = pDevice->CreateBuffer(&desc, &data, &pVertexBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"FAILED TO CREATE VERTEX BUFFER");
		assert(false);
	}


	// set up  the index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = NUM_INDICES * sizeof(DWORD);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;

	hr = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pIndexBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"FAILED TO CREATE INDEX BUFFER");
		assert(false);
	}
}

// ------------------------------------------------------------------------------------
// Initialzes the shaders
// ------------------------------------------------------------------------------------
void IndexedPrimitive::InitializeShaders(ID3D11Device* pDevice)
{
	// Step 4
	// create and configure the shader
	pEffect = new BasicEffect(pDevice);
	pEffect->SetLightingEnabled(false);
	pEffect->SetTextureEnabled(false);
	pEffect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;
	pEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	// create the input layout
	HRESULT hr = pDevice->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		&pInputLayout);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create input layout");
		assert(0);
	}
}

// ------------------------------------------------------------------------------------
// Draw the IndexedPrimitive
// ------------------------------------------------------------------------------------
void IndexedPrimitive::Draw(ID3D11DeviceContext* pDeviceContext, const Matrix& world, const Matrix& view, const Matrix& projection)
{
	//	Set up the shader inputs and apply the shader
	//
	pEffect->SetWorld(world);
	pEffect->SetView(view);
	pEffect->SetProjection(projection);
	pEffect->Apply(pDeviceContext);

	// Set up our input layout
	pDeviceContext->IASetInputLayout(pInputLayout);

	//  tell D3D we are drawing a triangle list
	pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//  Tell the device which vertex buffer we are using
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

	// Set the index buffer
	pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//	tell it to draw the primitive
	pDeviceContext->DrawIndexed(NUM_INDICES, 0, 0);
}


