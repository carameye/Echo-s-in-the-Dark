#pragma once

#include "common.hpp"
#include <vector>

class Circle;
class Square;

// Circle shape with collision detection
class Circle
{
public:
	// Constructor
	Circle(vec2 centre, int radius);

	Circle();

	// Return true if this collides with the given circle
	bool collides_with(Circle &circle);

	// Return true if this collides with the given square
	bool collides_with(Square &square);

	// Translates the circle
	void translate(vec2 translation);

private:
	// Position of the centre of the circle
	vec2 centre;

	// Radius of the circle
	int radius;
};

// Square shape with collision detection
class Square
{
public:
	// Constructor
	Square(vec2 bottomLeft, int width);

	Square();

	// Return true if this collides with the given circle
	bool collides_with(Circle &circle);

	// Return true if this collides with the given square
	bool collides_with(Square &square);

	// Translates the square
	void translate(vec2 translation);

	// Get the left most x coordinate of the square
	float get_left();

	// Get the right most x coordinate of the square
	float get_right();

	// Get the top most y coordinate of the square
	float get_top();

	// Get the bottom most y coordinate of the square
	float get_bottom();

private:
	// Position of the bottom left vertex of the square
	vec2 bottomLeft;

	// Width of the square
	int width;
};

// Collection of circles and squares with collision detection
class Hitbox
{
public:
	// Constructor
	Hitbox(std::vector<Circle> circles, std::vector<Square> squares);

	Hitbox();
	
	// Returns true if this collides with the given hitbox
	bool collides_with(Hitbox obj);

	// Translates the entire hitbox
	void translate(vec2 translation);

private:
	// Returns true if this collides with the given circle
	bool collides_with(Circle& circle);

	// Returns true if this collides with the given square
	bool collides_with(Square& square);

	// Collection of circles in the hitbox
	std::vector<Circle> circles;

	// Collection of squares in the hitbox
	std::vector<Square> squares;
};