#pragma once
#include <string>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include <VertexTypes.h>

// import the DirectXTK input layout and structure
using DirectX::VertexPositionColor;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class OBJLoader
{
public:

    bool Load(const std::string& filename, std::vector<VertexPositionColor>& vertices, std::vector<DWORD>& indices);

    bool LoadFromDialog(HWND hwnd, std::vector<VertexPositionColor>& verticesOut, std::vector<DWORD>& indicesOut);

};