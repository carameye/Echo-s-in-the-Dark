// Header
#include "robot.hpp"

#include <cmath>

Texture Robot::robot_texture;

bool Robot::init()
{
	// Load shared texture
	if (!robot_texture.is_valid())
	{
		if (!robot_texture.load_from_file(textures_path("body.png")))
		{
			fprintf(stderr, "Failed to load robot texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture.
	float wr = robot_texture.width * 0.5f;
	float hr = robot_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f,  };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { 0.f, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.position = { 0.f, 0.f };
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f , 0.f };
	motion.radians = 0.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = { 1.0f, 1.0f };

	return true;
}

// Releases all graphics resources
void Robot::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Robot::update(float ms)
{
	// TODO: handle  key strokes from world
	motion.radians += motion.velocity.x / 1000;
}

void Robot::draw(const mat3& projection, const vec2& camera_shift)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.rotate(motion.radians);
	vec2 tex_scale = { brick_size.x / robot_texture.width, brick_size.y / robot_texture.height };
	transform.scale(tex_scale);
	transform.end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, robot_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.out);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Robot::get_position() const
{
	return motion.position;
}

vec2 Robot::get_velocity() const
{
	return motion.velocity;
}

vec2 Robot::get_acceleration() const
{
	return motion.acceleration;
}

void Robot::set_position(vec2 position)
{
	motion.position = position;
}

void Robot::set_velocity(vec2 velocity)
{
	motion.velocity = velocity;
}

void Robot::set_acceleration(vec2 acceleration)
{
	motion.acceleration = acceleration;
}

Hitbox Robot::get_hitbox(vec2 translation) const
{
	std::vector<Circle> circles(1);
	
	vec2 position = motion.position;

	position.x += translation.x;
	position.y += translation.y;

	int radius = 33;
	Circle circle(position, radius);
	circles[0] = circle;

	Hitbox hitbox(circles, {});
	// TODO: figure out why this line is not working
	// hitbox.translate(translation);
	return hitbox;
}