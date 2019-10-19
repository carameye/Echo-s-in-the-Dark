/******************************************************************
When parsing a level, generate a graph:

	1. Based on the level data, generate a node for each critical point.
	2. For each pair of nodes, connect them with an edge if it is 
	   possible to travel straight between them without causing any 
	   collisions. We check this by incrementally moving along the path
	   and testing that our outer edges are not colliding with any 
	   bricks.
	3. Save this graph.

Then whenever we need to find a path between some start position and 
end position:

	1. If either we do not currently have a path to follow or if the 
	   robot has moved enough from its position when we last generated 
	   a path, generate a path to the robot.
	2. Create nodes at the start and end positions and add them to the 
	   graph, connecting them with other nodes by the same logic as 
	   before.
	3. Run A* path finding using a priority queue starting at the start
	   position searching for the end position. Return the result.

Then we use this path on each update by:

	1. Setting some distance that we would like our ghost to move per 
	   update.
	2. If the distance to the next element on the path is greater than 
	   what we are allowed to move, moving our full allowance in 
	   towards that node.
	3. Otherwise, moving completely to that node, decreasing our 
	   allowed movement by the distance we just moved, and looping back
	   to step 2.

******************************************************************/

#pragma once

#include "common.hpp"
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <SDL.h>
#undef main
#include <algorithm>
#include <math.h>


class CriticalPoint;

typedef std::pair<float, CriticalPoint*>  Edge;

class CriticalPoint
{
public:
	// Constructors
	CriticalPoint();
	CriticalPoint(vec2 loc);

	// Adds an edge
	void add_edge(CriticalPoint* cp);

	// Removes an edge
	void remove_edge(CriticalPoint* x);

	// Gets the edges connected to this vertex
	std::vector<Edge> get_edges();

	// Gets this vertex's id
	int get_id();

	// Gets position
	vec2 get_position();

private:
	int m_id;
	vec2 m_position;
	std::vector<Edge> m_edges;
};

class LevelGraph
{
public:
	// Constructor
	LevelGraph();

	// Given a string with map data, generates a graph
	bool generate(std::vector<vec2> cps, std::vector<std::vector<bool>> data, int width, int height);

	// Gets shortest path from start to goal
	// Uses A* search on level graph
	std::vector<vec2> get_path(vec2 start, vec2 goal);

private:
	std::vector<CriticalPoint> m_vertices;
	std::vector<std::vector<bool>> m_data;

	// Test if object can travel between two positions with no collisions
	bool can_travel_between(CriticalPoint a, CriticalPoint b);

	// Temporary modification
	void add_critical_point(CriticalPoint* cp);
	void remove_critical_point(CriticalPoint* cp);
};
