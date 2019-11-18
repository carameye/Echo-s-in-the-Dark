#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <map>
#include <utility>

// glfw
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// Simple utility macros to avoid mistyping directory name, name has to be a string literal
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "project_path.hpp"

#define shader_path(name) PROJECT_SOURCE_DIR "./shaders/" name

#define data_path PROJECT_SOURCE_DIR "./data"
#define textures_path(name)  data_path "/textures/" name
#define audio_path(name) data_path  "/audio/" name
#define mesh_path(name) data_path  "/meshes/" name
#define level_path data_path "/levels/json/"
#define shadow_path data_path "/levels/shadow/"
#define save_file data_path "/save/save_file.json"
#define maker_file level_path "maker_level.json"

enum class Status { nothing, main_menu, new_game, load_game, resume, reset, save_game, exit,
					story_mode, maker_mode, play_level, make_level};

// Not much math is needed and there are already way too many libraries linked (:
// If you want to do some overloads..
struct vec2 { float x, y; };
inline bool operator==(const vec2& lhs, const vec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
struct vec3 { float x, y, z; };
struct mat3 { vec3 c0, c1, c2; };

// Utility functions
float dot(vec2 l, vec2 r);
float dot(vec3 l, vec3 r);
mat3 mul(const mat3& l, const mat3& r);
vec2 mul(vec2 a, float b);
vec3 mul(mat3 m, vec3 v);
vec2 normalize(vec2 v);
vec2 add(vec2 a, vec2 b);
vec2 sub(vec2 a, vec2 b);
vec2 to_vec2(vec3 v);
vec2 vpow(vec2 v, float e); 
float sq_len(vec2 a);
float len(vec2 a);

vec2 to_grid_position(vec2 pos);
vec2 to_pixel_position(vec2 pos);

static const float brick_size = 64.f;
static const float TOLERANCE = 0.005f;

float get_closest_point(float last_pos, float tile_pos, float circle_width, float tile_width);
bool within_range(float val, float low, float high);

// OpenGL utilities
// cleans error buffer
void gl_flush_errors();
bool gl_has_errors();

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct Vertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Texture wrapper
struct Texture
{
	Texture();
	~Texture();

	GLuint id;
	GLuint depth_render_buffer_id;
	int width;
	int height;
	
	// Loads texture from file specified by path
	bool load_from_file(const char* path);
	bool is_valid()const; // True if texture is valid
	bool create_from_screen(GLFWwindow const * const window); // Screen texture
};

// A Mesh is a collection of a VertexBuffer and an IndexBuffer. A VAO
// represents a Vertex Array Object and is the container for 1 or more Vertex Buffers and 
// an Index Buffer.
struct Mesh {
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
};

// Effect component of Entity for Vertex and Fragment shader, which are then put(linked) together in a
// single program that is then bound to the pipeline.
struct Effect {
	GLuint vertex;
	GLuint fragment;
	GLuint program;

	bool load_from_file(const char* vs_path, const char* fs_path); // load shaders from files and link into program
	void release(); // release shaders and program
};

// All data relevant to the motion of the salmon.
struct Motion {
	vec2 position;
	vec2 velocity;
	vec2 acceleration;
	float radians;
};

// Scale is used in the bounding box calculations, 
// and so contextually belongs here (for now).
struct Physics {
	vec2 scale;
};

// Transform component handles transformations passed to the Vertex shader.
// gl Immediate mode equivalent, see the Rendering and Transformations section in the
// specification pdf.
struct Transform {
	mat3 out;

	void begin();
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
	void end();
};

// An entity boils down to a collection of components,
// organized by their in-game context (mesh, effect, motion, etc...)
struct Entity {
	int m_id;
};
