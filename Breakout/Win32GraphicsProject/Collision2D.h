// 
// Collision 
//		Handles 2D Collisions between simple shapes
//
// BTGD 9201
//


#ifndef _COLLISION_H
#define _COLLISION_H
#include <d3d11.h>
#include <SimpleMath.h>
using DirectX::SimpleMath::Vector2;

// Basic Shapes we use for collision

// 	
//	An axis aligned bounding box
//	
struct Box2D
{
	// Default constructor
	Box2D()
	{
		center = Vector2::Zero;
		extents = Vector2::Zero;
	}

	Box2D(Vector2 _center, Vector2 _extents)
	{
		center = _center;
		extents = _extents;
	}

	Vector2 center;
	Vector2 extents;
};


//
// Circle, represented by a center and radius
//
struct Circle
{
	Circle() { center = Vector2::Zero; radius = 0.0f; }
	Circle(Vector2 _center, float _radius)
	{
		center = _center;
		radius = _radius;
	}

	Vector2 center;
	float	radius;
};

//
// Line segment structure
//
struct Line2D
{
	Vector2 start;
	Vector2 end;

	Line2D(Vector2 _start, Vector2 _end)
	{
		start = _start;
		end = _end;
	}
	Line2D()
	{
		start = Vector2::Zero;
		end = Vector2::Zero;
	}
};

//
// A collection of 2D collision functions to test for collisions between different shapes
//
class Collision2D
{
public:
	// 
	static bool BoxCircleCheck(Box2D box, Circle circle);
	static bool BoxBoxCheck(Box2D boxA, Box2D boxB);
	static bool CircleCircleCheck(Circle circleA, Circle circleB);

	// Line / Line test
	//	returns true of the lines intersect (i.e. not parallel)
	//	t_a, t_b are the parameterized equation values
	static bool LineLineCheck(Line2D a, Line2D b, float& t_a, float& t_b, Vector2& intersection);

	// reflect a circle off a box - returns
	// the new position of the circle and updates the velocity
	static Vector2 ReflectCircleBox(Circle circle, Vector2& velocity, float deltaTime, Box2D box);

};



#endif // _COLLISION_H