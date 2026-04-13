//
// 2D Texture class
//
//	BTGD 9201 - This class will encapsulate loading and drawing textures to the screen
//
//

#ifndef _TEXTURE_TYPE_H
#define _TEXTURE_TYPE_H

#include <string>
#include <d3d11_1.h>

class TextureType 
{
public:
	// construction/destruction
	TextureType();
	~TextureType() { Unload(); }

	// loads the texture from disk
	bool Load( ID3D11Device* device, const wchar_t* fileName  );
	void Unload();

	// draws the texture to another 'resource'. Typically, drawTo will be the back buffer
	void Draw( ID3D11DeviceContext* device, ID3D11Texture2D* drawTo, int destX, int destY );

	// get height & width of the texture
	int GetHeight() const { return desc.Height; }
	int GetWidth() const { return desc.Width; }

	// get the resource view
	ID3D11ShaderResourceView* GetResourceView() const { return pView; }

private:

	ID3D11Texture2D*			pTexture;		// the directX interface to the texture
	ID3D11ShaderResourceView*	pView;			// our description when using with shaders

	std::wstring filePath;						// filename of the textures

	D3D11_TEXTURE2D_DESC desc;					// description of our texture 

	
};



#endif