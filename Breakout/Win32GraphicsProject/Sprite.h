//
// Sprite class - represents a single sprite object
//
//	BTGD 9201 - This class will encapsulate everything needed to draw a sprite
//


#ifndef _SPRITE_H
#define _SPRITE_H

#include <Windows.h>	// for RECT
#include <d3d11.h>
#include <SimpleMath.h> // for vectors and colours
#include <math.h>


// forward declares
class TextureType;
namespace DirectX { class SpriteBatch; }

// namespace resolution
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Color;
using DirectX::SpriteBatch;


// ----------------------------------------------------

class Sprite
{
public:

	// enumerated type representing the pivot
	enum Pivot
	{
		UpperLeft,
		UpperRight,
		Center,
		LowerLeft,
		LowerRight
	};

	// constructor
	Sprite();

	// initialize the sprite
	void Initialize(TextureType* pTexture, Vector2 position, float rotinDegrees, float scale, Color color, float layer);

	// set the pivot of the sprite
	void SetPivot(Pivot pivot);

	// set the texture region
	//  note - Initialize will reset this to the full texture
	void SetTextureRegion(int left, int top, int right, int bottom);

	// draw 
	void Draw(SpriteBatch* pBatch);

	// Get the sprite width and height, independent of rotation, scaled and rounded up
	int GetWidth() const	{ return (int)ceilf((textureRegion.right - textureRegion.left) * scale); }
	int GetHeight() const	{ return (int)ceilf((textureRegion.bottom - textureRegion.top) * scale); }

	// get the extents
	Vector2 GetExtents() const;

	void SetVelocity(Vector2 velocityPixPerSec, float rotationalVelocityDegPerSec); 
	Vector2 GetVelocity() const { return velocity; }
	float GetRotationalVelocity() { return rotationalVelocity * 180.0f / 3.141592f; }


	// check if this sprite contains this point 
	bool ContainsPoint(Vector2 pos) const;

	// get and set the color
	Color GetColor() const {	return color;	}
	void SetColor(Color c) { color = c; }
	
	// get and set the position
	Vector2 GetPosition() const { return position; }
	void SetPosition(Vector2 p) { position = p;  }

	// get and set the rotation in degrees
	float GetRotation() { return rotation * 180.0f / 3.141592f; }
	void SetRotation(float d) { rotation = d * 3.141592f / 180.0f;  }

	// get/set the scale
	float GetScale() const { return scale;  }
	void SetScale(float f) { scale = f;  }

	// Set up a texture animation
	void SetTextureAnimation(int frameSizeX, int frameSizeY, int framesPerSecond);

	// advance the animation
	void UpdateAnimation(float deltaTime);

	// check if we are on the last frame of animation
	bool isLastFrame() const { return currentFrame == totalFrames - 1; }

	// restart the animation
	void RestartAnimation() { currentFrame = 0;  }


private:
	// transformation information
	Vector2			position;
	float			rotation;
	float			scale;
	float			layer;

	// colour & texture
	Color			color;
	TextureType*	pTexture;

	// pivot/origin information
	Pivot			pivot;
	Vector2			origin;

	// the region of the texture we are drawing
	RECT			textureRegion;

	// get the center position of the sprite (internal helper)
	Vector2			GetCenterNoRotation() const;

	// texture animation
	int				frameWidth;
	int				frameHeight;
	int				totalFrames;
	int				currentFrame;

	float			elapsedTime;
	float			frameTime; // seconds per frames 

	// Set the texture animation region
	void SetTextureAnimationRegion();

	Vector2			velocity;		 // pixels /sec
	float			rotationalVelocity; // rad / sec

};


#endif 

