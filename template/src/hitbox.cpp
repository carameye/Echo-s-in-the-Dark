#include "hitbox.hpp"
#include <math.h>

Hitbox::Hitbox(std::vector<Circle> circles, std::vector<Square> squares)
{
	this->circles = circles;
	this->squares = squares;
}

Hitbox::Hitbox()
{

}

bool Hitbox::collides_with(Hitbox hb)
{
	for (Circle c : this->circles)
		if (hb.collides_with(c))
			return true;

	for (Square s : this->squares)
		if (hb.collides_with(s))
			return true;

	return false;
}

void Hitbox::translate(vec2 translation)
{
	for (Circle& c : this->circles)
	{
		c.translate(translation);
	}

	for (Square& s : this->squares)
	{
		s.translate(translation);
	}
}

bool Hitbox::collides_with(Circle &circle)
{
	for (Circle c : this->circles)
		if (circle.collides_with(c))
			return true;

	for (Square s : this->squares)
		if (circle.collides_with(s))
			return true;

	return false;
}

bool Hitbox::collides_with(Square& square)
{
	for (Circle c : this->circles)
		if (square.collides_with(c))
			return true;

	for (Square s : this->squares)
		if (square.collides_with(s))
			return true;

	return false;
}

Circle::Circle(vec2 centre, int radius)
{
	this->centre = centre;
	this->radius = radius;
}

Circle::Circle()
{

}

bool Circle::collides_with(Circle &circle)
{
	return len(sub(circle.centre, this->centre)) <= circle.radius + this->radius + TOLERANCE;
}

bool Circle::collides_with(Square &square)
{
	float testX = this->centre.x;
	float testY = this->centre.y;
	float sxMin = square.get_left();
	float sxMax = square.get_right();
	float syMin = square.get_top();
	float syMax = square.get_bottom();

	if (this->centre.x < sxMin)
		testX = sxMin;
	else if (this->centre.x > sxMax) 
		testX = sxMax;
	if (this->centre.y < syMin)
		testY = syMin;
	else if (this->centre.y > syMax) 
		testY = syMax;

	float distX = this->centre.x - testX;
	float distY = this->centre.y - testY;
	float distance = sqrt((distX * distX) + (distY * distY));

	return distance <= this->radius + TOLERANCE;
}

void Circle::translate(vec2 translation)
{
    vec2 new_centre = add(this->centre, translation);
	this->centre = new_centre;
}

Square::Square(vec2 bottomLeft, int width)
{
	this->bottomLeft = bottomLeft;
	this->width = width;
}

Square::Square()
{
	
}

bool Square::collides_with(Circle &circle)
{
	return circle.collides_with(*this);
}

bool Square::collides_with(Square &square)
{
	bool xOverlap = this->get_left() <= square.get_right() + TOLERANCE
		&& this->get_right() + TOLERANCE >= square.get_left();
	bool yOverlap = this->get_top() <= square.get_bottom()
		&& this->get_bottom() + TOLERANCE >= square.get_top() + TOLERANCE;

	return xOverlap && yOverlap;
}

void Square::translate(vec2 translation)
{
	this->bottomLeft = add(this->bottomLeft, translation);
}

float Square::get_left()
{
	return this->bottomLeft.x;
}

float Square::get_right()
{
	return this->bottomLeft.x + this->width;
}

float Square::get_top()
{
	return this->bottomLeft.y - this->width;
}

float Square::get_bottom()
{
	return this->bottomLeft.y;
}