//
// Sprite class - represents a single sprite object
//
//	BTGD 9201 - This class will encapsulate everything needed to draw a sprite
//

#include "Sprite.h"
#include <SpriteBatch.h>
#include <DirectXColors.h>
#include "TextureType.h"
#include <math.h>
using namespace DirectX::SimpleMath;

// constant
const float twoPi = 3.141592f * 2.0f;

// convert degrees to radians
inline float DegToRad( float deg ) { return 3.141592f * deg / 180.0f; } 

// clamp an integer
inline int Clamp( int v, int min, int max ) 
{
	if ( v < min ) v = min;
	if ( v > max ) v = max;
	return v;
}

// -----------------------------------------------------------------------------
// constructor
Sprite::Sprite()
{
	position = Vector2(0,0);
	rotation = 0;
	scale = 1;

	color = Color(DirectX::Colors::White.v);
	pTexture = NULL;

	pivot = Sprite::Center;
	origin = Vector2(0,0);

	layer = 0.0f;

	textureRegion.left = 0;
	textureRegion.top = 0;
	textureRegion.bottom = 0;
	textureRegion.right = 0;

	frameWidth = 0;
	frameHeight = 0;
	totalFrames = 0;
	currentFrame = 0;
	elapsedTime = 0;
	frameTime = 0;
}

// -----------------------------------------------------------------------------
// initialize the sprite
void Sprite::Initialize( TextureType* pTex, Vector2 pos, float rotInDegrees, float scl, Color clr, float lyr ) 
{
	pTexture = pTex;
	position = pos;
	rotation = DegToRad( rotInDegrees );
	scale = scl;
	color = clr;
	layer = lyr;

	if ( pTexture )
	{
		textureRegion.left = 0;
		textureRegion.top = 0;
		textureRegion.bottom = pTex->GetHeight();
		textureRegion.right = pTex->GetWidth();
	}

	// updates the pivot based on the texture size
	SetPivot( pivot );
}

// -----------------------------------------------------------------------------
// set the pivot of the sprite
void Sprite::SetPivot( Sprite::Pivot pvt )
{
	pivot = pvt; // save the value

	if ( pTexture != NULL )
	{
		switch(pivot)
		{
		case Sprite::UpperLeft: 
			origin = Vector2( float(textureRegion.left), float(textureRegion.top) );
			break;
		case Sprite::UpperRight:
			origin = Vector2( float(textureRegion.right), float(textureRegion.top) );
			break;
		case Sprite::Center:
			origin = Vector2( float(textureRegion.right - textureRegion.left)/2.0f, float(textureRegion.bottom - textureRegion.top)/2.0f );
			break;
		case Sprite::LowerRight:
			origin = Vector2( float(textureRegion.right), float(textureRegion.bottom) );
			break;
		case Sprite::LowerLeft:
			origin = Vector2( float(textureRegion.left), float(textureRegion.bottom) );
			break;
		}
	}
}

// -----------------------------------------------------------------------------
// draw - Requires pBatch->Begin() to be called prior to this
void Sprite::Draw( SpriteBatch* pBatch )
{
	if ( pTexture )
	{
		pBatch->Draw( pTexture->GetResourceView(), position, &textureRegion ,color, rotation, origin, scale, DirectX::SpriteEffects_None, layer );
	}
}

// -----------------------------------------------------------------------------
// Sets the texture to show only a portion of the texture
//
void Sprite::SetTextureRegion( int left, int top, int right, int bottom )
{
	// verify it's within the texture region
	if ( pTexture != NULL )
	{
		left = Clamp( left, 0, pTexture->GetWidth() );
		top = Clamp( top, 0, pTexture->GetHeight() );
		right = Clamp( right, 0, pTexture->GetWidth() );
		bottom = Clamp( bottom, 0, pTexture->GetHeight() );
	}

	textureRegion.left = left;
	textureRegion.right = right;
	textureRegion.top = top;
	textureRegion.bottom = bottom;

	// reset the pivot
	SetPivot( pivot );
}

// -----------------------------------------------------------------------------
//  Returns true if the given point is within the bounds of the sprites
// 
bool Sprite::ContainsPoint(Vector2 point) const
{
	float halfwidth = 0.5f*(float) GetWidth();
	float halfheight = 0.5f*(float)GetHeight();
	
	// get the pivot point of the sprite
	Vector2 center = GetCenterNoRotation();

	if (rotation != 0.0f)
	{
		// rotate the point in the opposite direction
		Matrix rotMatrix = Matrix::CreateRotationZ(-rotation);
		Vector3 dir(point.x - position.x, point.y - position.y, 0.0f);
		dir = Vector3::TransformNormal(dir, rotMatrix);

		// update the point to a new position in the space of the unrotated box
		point = center - dir;
	}

	return point.x >= (center.x - halfwidth) &&
		point.x <= (center.x + halfwidth) &&
		point.y >= (center.y - halfheight) &&
		point.y <= (center.y + halfheight);
		

}

// -----------------------------------------------------------------------------
// get the center position of the sprite. does not account for rotation
Vector2	Sprite::GetCenterNoRotation() const
{
	float halfwidth = 0.5f * (float)GetWidth();
	float halfheight = 0.5f * (float)GetHeight();

	switch (pivot)
	{
	case Sprite::UpperLeft:
		return position + Vector2(-halfwidth, -halfheight);
	case Sprite::UpperRight:
		return position + Vector2(-halfwidth, -halfheight);
	case Sprite::Center:
		return position;
	case Sprite::LowerRight:
		return position + Vector2(-halfwidth, -halfheight);
	case Sprite::LowerLeft:
		return position + Vector2(-halfwidth, -halfheight);
	}
	return position;
}

// --------------------------------------------------------------------
// Set up texture animation parameters
//  assumes that the full sheet is used
void Sprite::SetTextureAnimation(int frameSizeX, int frameSizeY, int framesPerSecond)
{
	if (pTexture == nullptr)
		return;

	frameWidth = frameSizeX;
	frameHeight = frameSizeY;
	int cols = pTexture->GetWidth() / frameWidth;
	int rows = pTexture->GetHeight() / frameHeight;
	totalFrames = rows * cols;
	currentFrame = 0;

	elapsedTime = 0;
	frameTime = 1.0f / framesPerSecond;

	SetTextureAnimationRegion();
}

// --------------------------------------------------------------------
// Updates the animation
void Sprite::UpdateAnimation(float deltaTime)
{
	if (totalFrames > 0)
	{
		elapsedTime += deltaTime;

		// how many frames to move forward
		int advanceFrames = int(elapsedTime / frameTime); 
		// fmodf is the % operator for floats
		elapsedTime = fmodf(elapsedTime, frameTime);

		// advance the animation
		//  % operator causes it to wrap to 0
		currentFrame = (currentFrame + advanceFrames) % totalFrames;

		// recalculate the region
		SetTextureAnimationRegion();
	}

	position += velocity * deltaTime;
	rotation += rotationalVelocity * deltaTime;

	// keep it honest
	while (rotation > twoPi) rotation -= twoPi;
	while (rotation < -twoPi) rotation += twoPi;
}


// ------------------------------------------------------------
// Sets the texture region based on the current animation frame
//
void Sprite::SetTextureAnimationRegion()
{
	// get the number of columns
	int cols = pTexture->GetWidth() / frameWidth;

	// get the column and the row
	int c = currentFrame % cols;
	int r = currentFrame / cols;

	// set the texture region
	textureRegion.left = c * frameWidth;
	textureRegion.right = (c + 1) * frameWidth;
	textureRegion.top = r * frameHeight;
	textureRegion.bottom = ( r + 1 )  * frameHeight;

	// we've change the texturegion, update the origin
	SetPivot(pivot);
}


// -----------------------------------------------------------------------
// Get the extents of the bounding box of the sprite 
//	Extents are 1/2 width and height of the transformed sprite
Vector2 Sprite::GetExtents() const 
{

	Vector2 extents(
		(textureRegion.right - textureRegion.left) * scale * 0.5f,
		(textureRegion.bottom - textureRegion.top) * scale * 0.5f
		);

	// need to adjust it for rotation
	if (rotation != 0)
	{
		float cosTheta = cosf(rotation);
		float sinTheta = sinf(rotation);

		Vector2 rotated = Vector2(
				extents.x * cosTheta  - extents.y * sinTheta,
				extents.x * sinTheta  + extents.y * cosTheta
			);
			
		extents.x = fabsf(rotated.x);
		extents.y = fabsf(rotated.y);
	}

	return extents;
}

// -----------------------------------------------------------------------
// Sets the velocity
//
void Sprite::SetVelocity(Vector2 velocityPixPerSec, float rotationalVelocityDegPerSec)
{
	velocity = velocityPixPerSec;
	rotationalVelocity = DegToRad(rotationalVelocityDegPerSec);
}
