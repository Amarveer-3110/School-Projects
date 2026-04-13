#pragma once

#include <unordered_map>
#include <string>
#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class TransformLoader
{
public:

    bool Load(const std::string& filename);
    Matrix GetMatrix(const std::string& index) const;
    bool TransformLoader::LoadFromDialog(HWND hwnd);

    int GetCount() const;

private:

    std::unordered_map<std::string, Matrix> matrices;

    Matrix ParseMatrix(std::ifstream& file);
};