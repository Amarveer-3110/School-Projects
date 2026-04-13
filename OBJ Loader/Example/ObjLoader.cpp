#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

// ------------------------------------------------------------
// Load OBJ file
// ------------------------------------------------------------
bool OBJLoader::Load(const std::string& filename, std::vector<VertexPositionColor>& verticesOut, std::vector<DWORD>& indicesOut)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::vector<Vector3> positions;
    verticesOut.clear();
    indicesOut.clear();

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v")
        {
            float x, y, z;
            if (!(ss >> x >> y >> z))
                continue;

            positions.push_back(Vector3(x, y, z));
        }
        else if (type == "f")
        {
            std::string tok1, tok2, tok3;
            if (!(ss >> tok1 >> tok2 >> tok3))
                continue;

            auto parseIndex = [](const std::string& token) -> DWORD
                {
                    std::string first = token;
                    size_t slash = first.find('/');
                    if (slash != std::string::npos)
                        first = first.substr(0, slash);

                    int idx = atoi(first.c_str());
                    if (idx <= 0)
                        return 0;

                    return (DWORD)(idx - 1);
                };

            DWORD i1 = parseIndex(tok1);
            DWORD i2 = parseIndex(tok2);
            DWORD i3 = parseIndex(tok3);

            if (i1 >= positions.size() || i2 >= positions.size() || i3 >= positions.size())
                continue;

            DWORD base = (DWORD)verticesOut.size();
            verticesOut.push_back(VertexPositionColor(positions[i1], Color(1.0f, 1.0f, 1.0f, 1.0f)));
            verticesOut.push_back(VertexPositionColor(positions[i2], Color(1.0f, 1.0f, 1.0f, 1.0f)));
            verticesOut.push_back(VertexPositionColor(positions[i3], Color(1.0f, 1.0f, 1.0f, 1.0f)));

            indicesOut.push_back(base + 0);
            indicesOut.push_back(base + 1);
            indicesOut.push_back(base + 2);
        }
    }

    return true;
}

bool OBJLoader::LoadFromDialog(HWND hwnd, std::vector<VertexPositionColor>& verticesOut, std::vector<DWORD>& indicesOut)
{
    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "obj";
    ofn.lpstrInitialDir = currentDir;

    if (GetOpenFileNameA(&ofn))
    {
        return Load(fileName, verticesOut, indicesOut);
    }

    return false;
}