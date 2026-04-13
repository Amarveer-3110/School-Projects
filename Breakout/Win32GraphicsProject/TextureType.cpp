//
// 2D Texture class
//
//

#include "TextureType.h"
#include "DirectX.h"
#include <WICTextureLoader.h> // for loading bmp, jpgs
#include <DDSTextureLoader.h> // for loading dds files

// ----------------------------------------------------------
// Constructor 
//
TextureType::TextureType()
{
	pView = NULL;
	pTexture = NULL;
}

// ----------------------------------------------------------
// Load the texture from disk
//
bool TextureType::Load( ID3D11Device* device,  const wchar_t* fileName  )
{
	// If we're already loaded, unload the previous
	if ( pTexture != NULL ) 
	{
		Unload();
	}

	// save the path to the file
	filePath = fileName;

	// error code if one occurs
	HRESULT result;

	// check if it's a dds file or not
	if ( filePath.find(L".dds") != std::wstring::npos )
	{
		result = DirectX::CreateDDSTextureFromFile( device, fileName, (ID3D11Resource**) &pTexture, &pView );
	}
	else
	{
		result = DirectX::CreateWICTextureFromFile( device, fileName, (ID3D11Resource**) &pTexture, &pView );
	}

	// check if we loaded the file
	if ( result != S_OK )
	{
		return false;
	}

	// get the information about the texture
	pTexture->GetDesc( &desc );

	return true;
}

// ----------------------------------------------------------
// draws the texture to another 'resource'. Typically, drawTo will be the back buffer
void TextureType::Draw( ID3D11DeviceContext* device, ID3D11Texture2D* drawTo, int destX, int destY )
{
	// if we aren't loaded
	if ( pTexture == NULL )
	{
		return;
	}

	// get the width and height of what we are drawing to
	D3D11_TEXTURE2D_DESC toDesc;
	drawTo->GetDesc( &toDesc );

	// width of the surface we are drawing to
	int toWidth = (int) toDesc.Width;
	int toHeight = (int) toDesc.Height;

	// if we are offscreen, don't draw
	if ( destX >= toWidth || destY >= toHeight || destX <= -toWidth || destY <= -toHeight )
	{
		return;
	}

	int width = desc.Width;
	int height = desc.Height;

	int left = 0;
	int top = 0;

	// if we are offscreen, reduce our width
	if ( destX < 0 ) 
	{
		left = -destX;
		width = width + destX;
	}
	if ( destY < 0 )
	{
		top = -destY;
		height = height + destY;
	}

	// check if we are off the edge of the right screen
	if ( destX + width > (int) toDesc.Width )
	{
		width = (int)toDesc.Width - destX;
	}

	if ( destY + height > (int) toDesc.Height )
	{
		height = (int)toDesc.Height - destY;
	}


	// describe the sub area we want to draw to
	D3D11_BOX sourceRegion;					// box region

	sourceRegion.left = left;
	sourceRegion.right = width;
	sourceRegion.top = top;
	sourceRegion.bottom = height;
	sourceRegion.front = 0;
	sourceRegion.back = 1;

	// Copy part of a texture resource to the back buffer, or where- 
	device->CopySubresourceRegion( drawTo, 0, destX, destY, 0, pTexture, 0, &sourceRegion );

}

// ----------------------------------------------------------
// Unloads the texture
void TextureType::Unload()
{
	SAFE_RELEASE( pTexture );
	SAFE_RELEASE( pView );
}
