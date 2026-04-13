// 
// Collision2D
//		Handles 2D Collisions between simple shapes
//
// BTGD 9201
//

#include "Collision2D.h"

// -----------------------------------------------------
// Box/Box check
//
bool Collision2D::BoxBoxCheck(Box2D boxA, Box2D boxB)
{
	// TODO - Exercise fill this in
	Vector2 delta = boxA.center - boxB.center;
	delta.x = fabsf(delta.x);
	delta.y = fabsf(delta.y);

	if (delta.x < (boxA.extents.x + boxB.extents.x))
	{
		if (delta.y < (boxA.extents.y + boxB.extents.y))
			return true;
	}

	return false;
}


// -----------------------------------------------------
// Box Circle Check
//
bool Collision2D::BoxCircleCheck(Box2D box, Circle circle)
{
	Vector2 distance = circle.center - box.center;

	// check the x axis distance
	if (fabsf(distance.x) > (box.extents.x + circle.radius))
		return false;

	// check the y axis distance
	if (fabsf(distance.y) > (box.extents.y + circle.radius))
		return false;

	// straight line distance from 
	if (distance.Length() > (box.extents.Length() + circle.radius))
		return false;

	return true;
}


// -----------------------------------------------------
// Circle / Circle check
//
bool Collision2D::CircleCircleCheck(Circle circleA, Circle circleB)
{
	float distance = Vector2::Distance(circleA.center,circleB.center);
	return distance <= (circleA.radius + circleB.radius);
}

inline bool IsZeroToOne(float t) { return t >= 0 && t <= 1;  }

// -----------------------------------------------------
// reflect a circle off a box - returns
// the new position of the circle and updates the velocity
Vector2 Collision2D::ReflectCircleBox(Circle circle, Vector2& velocity, float deltaTime, Box2D box)
{
	float extra = circle.radius;

	// our first contact point is the edge of the circle, our last is based on our velocity
	Line2D  circleMovement;

	// line segments of the box
	Line2D top(box.center + Vector2(-box.extents.x - extra, -box.extents.y), box.center + Vector2(box.extents.x + extra, -box.extents.y));
	Line2D left(box.center + Vector2(-box.extents.x, -box.extents.y - extra), box.center + Vector2(-box.extents.x, box.extents.y + extra));
	Line2D right(box.center + Vector2(box.extents.x,-box.extents.y - extra), box.center + Vector2(box.extents.x, box.extents.y + extra));
	Line2D bottom(box.center + Vector2(-box.extents.x -extra, box.extents.y), box.center + Vector2(box.extents.x + extra, box.extents.y));


	// the normal of the edge we hit
	Vector2 normal = Vector2::Zero;

	float t = -10000; // the final time (0-1)

	float ty = 10000;
	float tx = 10000;

	// 
	float tCircle, tBox;
	Vector2 contact;

	// test for the box moving down
	if (velocity.y > 0)
	{
		circleMovement.start = circle.center + Vector2(0, circle.radius);
		circleMovement.end = circleMovement.start + velocity*deltaTime;

		LineLineCheck(circleMovement, top, tCircle, tBox, contact);
		if (IsZeroToOne(tCircle) && IsZeroToOne(tBox))
		{
			t = tCircle;
			normal = Vector2(0, -1); // reflect up
		}
		else if (IsZeroToOne(-tCircle) && IsZeroToOne(tBox)) // detect corner cases (t -1>circle>0)
		{
			ty = tCircle;
			normal.y = -1;
		}
	}
	// moving up
	else if (velocity.y < 0)
	{
		circleMovement.start = circle.center + Vector2(0, -circle.radius);
		circleMovement.end = circleMovement.start + velocity*deltaTime;

		LineLineCheck(circleMovement, bottom, tCircle, tBox, contact);
		if (IsZeroToOne(tCircle) && IsZeroToOne(tBox))
		{
			t = tCircle;
			normal = Vector2(0, 1); // reflect down
		}
		else if (IsZeroToOne(-tCircle) && IsZeroToOne(tBox)) // detect corner cases (t -1>circle>0)
		{
			ty = tCircle;
			normal.y = 1;
		}

	}

	if (velocity.x > 0)
	{
		circleMovement.start = circle.center + Vector2(circle.radius,0);
		circleMovement.end = circleMovement.start + velocity*deltaTime;

		LineLineCheck(circleMovement, left, tCircle, tBox, contact);
		if (IsZeroToOne(tCircle) && IsZeroToOne(tBox))
		{
			// make sure it's a closer collision
			if (t < 0 || tCircle < t)
			{
				t = tCircle;
				normal = Vector2(-1, 0); // reflect float left
			}
		}
		else if (IsZeroToOne(-tCircle) && IsZeroToOne(tBox)) // detect corner cases (t -1>circle>0)
		{
			tx = tCircle;
			normal.x = -1;
		}

	}
	else if (velocity.x < 0)
	{
		circleMovement.start = circle.center + Vector2(-circle.radius, 0);
		circleMovement.end = circleMovement.start + velocity*deltaTime;

		LineLineCheck(circleMovement, right, tCircle, tBox, contact);
		if (IsZeroToOne(tCircle) && IsZeroToOne(tBox))
		{
			// make sure it's a closer collision
			if (t < 0 || tCircle < t)
			{
				t = tCircle;
				normal = Vector2(1, 0); // reflect right
			}
		}
		else if (IsZeroToOne(-tCircle) && IsZeroToOne(tBox)) // detect corner cases (t -1>circle>0)
		{
			tx = tCircle;
			normal.x = 1;
		}
	}


	Vector2 finalPosition;

	// we were already penetrating on the axis because of the corner to reflect off accordingly 
	if (t < 0 && (tx < 0 || ty < 0))
	{
		t = 0;
		normal.Normalize();
	}


	// make sure we had a contact point
	if (t >= 0)
	{
		// get the new velocity by reflecting it in the normal
		Vector2 newVelocity;
		Vector2::Reflect(velocity, normal, newVelocity);

		// calculate the new center
		finalPosition = circle.center + velocity * t * deltaTime + newVelocity * (1 - t) * deltaTime;
	
		velocity = newVelocity;
	}
	else
	{
		// no intersection, move the circle
		finalPosition = circle.center + deltaTime * velocity;
	}

	return finalPosition;

}

// Line / Line test
//	returns true of the lines intersect (i.e. not parallel)
//	t_a, t_b are the parameter equation values
//  intersection is the intersection point
bool Collision2D::LineLineCheck(Line2D a, Line2D b, float& t_a, float& t_b, Vector2& intersection )
{
	// start and end points
	float x1 = a.start.x; float y1 = a.start.y;
	float x2 = a.end.x; float y2 = a.end.y;

	float x3 = b.start.x; float y3 = b.start.y;
	float x4 = b.end.x; float y4 = b.end.y;
	
	// calculate the denominator
	float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

	// parallel line check
	if (fabsf(denom) < 0.0001f)
		return false;

	// find the intersection point using the qui
	intersection.x =
		(x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4);
	intersection.y =
		(x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4);
	intersection *= 1 / denom;

	if (fabsf(x2 - x1) > fabsf(y2 - y1))
		t_a = (intersection.x - x1) / (x2 - x1);
	else
		t_a = (intersection.y - y1) / (y2 - y1);

	if (fabsf(x4 - x3) > fabsf(y4 - y3))
		t_b = (intersection.x - x3) / (x4 - x3);
	else
		t_b = (intersection.y - y3) / (y4 - y3);

	return true;
}


