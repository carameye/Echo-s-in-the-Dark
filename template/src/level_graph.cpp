#include "level_graph.hpp"
#include <queue>

static int v_id = 0;

CriticalPoint::CriticalPoint()
{
}

CriticalPoint::CriticalPoint(vec2 loc)
{
	m_id = v_id++;
	m_position = loc;
	m_edges.clear();
}

void CriticalPoint::add_edge(CriticalPoint* cp)
{
	m_edges.push_back(std::make_pair(len(sub(get_position(), cp->get_position())), cp));
}

void CriticalPoint::remove_edge(CriticalPoint* x)
{
	for (int i = 0; i < m_edges.size(); i++)
	{
		if (m_edges[i].second->get_id() == x->get_id())
		{
			m_edges.erase(m_edges.begin() + i);
			return;
		}
	}
}

std::vector<Edge> CriticalPoint::get_edges()
{
	return m_edges;
}

int CriticalPoint::get_id()
{
	return m_id;
}

vec2 CriticalPoint::get_position()
{
	return m_position;
}

LevelGraph::LevelGraph()
{
	m_vertices.clear();
}

static float h(CriticalPoint* u, vec2 goal)
{
	return len(sub(u->get_position(), goal));
}

std::vector<vec2> LevelGraph::get_path(const vec2 start, const vec2 goal)
{
	static vec2 s_goal = goal;
	CriticalPoint start_node(start);
	CriticalPoint goal_node(goal);

	add_critical_point(&start_node);
	add_critical_point(&goal_node);

	if (can_travel_between(start_node, goal_node))
	{
		start_node.add_edge(&goal_node);
		goal_node.add_edge(&start_node);
	}

	// We will use this a lot so simplify it
	typedef std::pair<float, std::vector<CriticalPoint*>> elem;

	// Make priority queue comparison function
	struct ComparePath {
		bool operator()(elem const& e1, elem const& e2)
		{ 
			return e1.first + h(e1.second.back(), s_goal)
				> e2.first + h(e2.second.back(), s_goal);
		}
	};

	// Initialize the queue
	std::priority_queue<elem, std::vector<elem>, ComparePath> frontier;

	std::vector<CriticalPoint*> v;
	v.push_back(&start_node);
	elem n = std::make_pair(0.f, v);
	frontier.push(n);

	float best_path_length = INFINITY;
	std::vector<CriticalPoint*> best_path;

	while (!frontier.empty())
	{
		elem element = frontier.top();
		frontier.pop();

		CriticalPoint* cp = element.second.back();

		if (element.first + h(cp, goal) > best_path_length)
		{
			continue;
		}

		if (cp->get_id() == goal_node.get_id())
		{
			best_path_length = element.first;
			best_path = element.second;
			continue;
		}

		for (auto& edge : cp->get_edges())
		{
			auto it = std::find(v.begin(), v.end(), edge.second);
			if (it != v.end())
			{
				continue;
			}

			v.push_back(edge.second);
			std::vector<CriticalPoint*> n_v = element.second;
			n_v.push_back(edge.second);
			elem n_e = std::make_pair(element.first + edge.first, n_v);
			frontier.push(n_e);
		}
	}

	remove_critical_point(&start_node);
	remove_critical_point(&goal_node);

	std::vector<vec2> path;

	for (int i = 0; i < best_path.size(); i++)
	{
		path.push_back(best_path[i]->get_position());
	}

	return path;
}

bool LevelGraph::generate(std::vector<vec2> cps, std::vector<std::vector<bool>> data, int width, int height)
{
	fprintf(stderr, "Generating graph\n");
	m_vertices.clear();
	m_data = data;

	int num_crits = 0;
	int num_edges = 0;

	std::vector<vec2> diffs = { { -1.f, 0.f }, { 1.f, 0.f }, { 0.f, 1.f }, { 0.f, -1.f }, { 0.f, 0.f } };

	for (vec2 cp : cps)
	{
		bool valid = true;
		for (vec2 diff : diffs)
		{
			vec2 pos = add(cp, diff);
			if (pos.x >= 0.f && pos.x < width && pos.y >= 0.f && pos.y < height)
			{
				valid &= !data[(int)pos.y][(int)pos.x];
			}
		}
		
		if (valid)
		{
			CriticalPoint v(to_pixel_position(cp));
			m_vertices.push_back(v);
			num_crits++;
		}
	}

	for (int i = 0; i < m_vertices.size(); i++)
	{
		for (int j = i + 1; j < m_vertices.size(); j++)
		{
			CriticalPoint* a = &m_vertices[i];
			CriticalPoint* b = &m_vertices[j];
			if (can_travel_between(*a, *b))
			{
				float distance = len(sub(a->get_position(), b->get_position()));
				a->add_edge(b);
				b->add_edge(a);
				num_edges++;
			}
		}
	}

	fprintf(stderr, "	generated graph with n=%d, m=%d\n", num_crits, num_edges);
	return true;
}

bool LevelGraph::can_travel_between(CriticalPoint a, CriticalPoint b)
{
	vec2 start = to_grid_position(a.get_position());
	vec2 finish = to_grid_position(b.get_position());
	vec2 disp = sub(finish, start);
	
	if (abs(disp.x) > abs(disp.y))
	{
		float xdist = 0.f;
		float max = abs(disp.x);
		float sign = disp.x / max;

		while (xdist < abs(disp.x))
		{
			float xfirst = floor(xdist) + TOLERANCE;
			float xlast = floor(xdist + 1.f + TOLERANCE) - TOLERANCE;

			float yfirst = add(start, mul(disp, xfirst / max)).y;
			float ylast = add(start, mul(disp, xlast / max)).y;

			if (m_data[(int)floor(yfirst + 1.f - TOLERANCE)][(int)floor(start.x + sign * xfirst)] ||
				m_data[(int)floor(yfirst       + TOLERANCE)][(int)floor(start.x + sign * xfirst)] ||
				m_data[(int)floor(ylast  + 1.f - TOLERANCE)][(int)floor(start.x + sign * xlast)] ||
				m_data[(int)floor(ylast        + TOLERANCE)][(int)floor(start.x + sign * xlast)])
			{
				return false;
			}

			float remaining = max - xdist;
			if (remaining - floor(remaining) > TOLERANCE)
				xdist += remaining - floor(remaining);
			else
				xdist++;
		}

		return true;
	}
	else
	{
		float ydist = 0.f;
		float max = abs(disp.y);
		float sign = disp.y / max;

		while (ydist < abs(disp.y))
		{
			float yfirst = floor(ydist) + TOLERANCE;
			float ylast = floor(ydist + 1.f + TOLERANCE) - TOLERANCE;

			float xfirst = add(start, mul(disp, yfirst / max)).x;
			float xlast = add(start, mul(disp, ylast / max)).x;

			if (m_data[(int)floor(start.y + sign * yfirst)][(int)floor(xfirst + 1.f - TOLERANCE)] ||
				m_data[(int)floor(start.y + sign * yfirst)][(int)floor(xfirst       + TOLERANCE)] ||
				m_data[(int)floor(start.y + sign * ylast)] [(int)floor(xlast  + 1.f - TOLERANCE)] ||
				m_data[(int)floor(start.y + sign * ylast)] [(int)floor(xlast        + TOLERANCE)])
			{
				return false;
			}

			float remaining = max - ydist;
			if (remaining - floor(remaining) > TOLERANCE)
				ydist += remaining - floor(remaining);
			else
				ydist++;
		}

		return true;
	}

	return false;
}

void LevelGraph::add_critical_point(CriticalPoint* cp)
{
	vec2 pos = cp->get_position();

	for (auto& v : m_vertices)
	{
		if (can_travel_between(*cp, v))
		{
			cp->add_edge(&v);
			v.add_edge(cp);
		}
	}
}

void LevelGraph::remove_critical_point(CriticalPoint* cp)
{
	for (auto& e : cp->get_edges())
	{
		CriticalPoint* v = e.second;
		v->remove_edge(cp);
	}
}
