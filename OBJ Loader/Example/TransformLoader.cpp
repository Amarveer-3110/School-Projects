#include "TransformLoader.h"
#include <fstream>
#include <sstream>

bool TransformLoader::Load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        // Object name (comment line)
        if (line[0] == '#')
        {
            //Remove '#'
            std::string name = line.substr(1);

            //Remove leading whitespace
            name.erase(0, name.find_first_not_of(" \t"));

            //Parse the matrix block below this name in the file:
            Matrix matrix = ParseMatrix(file);

            //Store it in the hashmap
            matrices[name] = matrix;
        }
    }

    return true;
}

Matrix TransformLoader::ParseMatrix(std::ifstream& file)
{
    std::string line;
    Matrix transform = Matrix::Identity;

    while (std::getline(file, line))
    {
        if (line.empty())
            break;

        std::stringstream ss(line);

        char type;
        ss >> type;

        //Scale Matrix:
        if (type == 's')
        {
            float x = 1, y = 1, z = 1;
            //Try to read one number from the steam.
            ss >> x;
            //Try to read two more numbers from the stream. If it succeeds we have a 3 axis scales.
            if (ss >> y >> z)
                transform *= Matrix::CreateScale(x, y, z);
            else
                transform *= Matrix::CreateScale(x);
        }
        //Transform Matrix
        else if (type == 't')
        {
            float x, y, z;
            ss >> x >> y >> z;

            transform *= Matrix::CreateTranslation(x, y, z);
        }
        //Rotation Matrix
        else if (type == 'r')
        {
            float x, y, z;
            ss >> x >> y >> z;

            transform *= Matrix::CreateFromYawPitchRoll(x, y, z);
        }
    }

    return transform;
}

bool TransformLoader::LoadFromDialog(HWND hwnd)
{
    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    //Get the current working directory
    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    //Options:
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "txt";    
    ofn.lpstrInitialDir = currentDir;

    //Open the dialogue window to search for a file from the user and get the file name
    if (GetOpenFileNameA(&ofn))
    {
        //Load the file
        matrices.clear();
        return Load(fileName);
    }

    return false;
}

Matrix TransformLoader::GetMatrix(const std::string& key) const
{
    return matrices.at(key);
}

int TransformLoader::GetCount() const
{
    return (int)matrices.size();
}