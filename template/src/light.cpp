#include "light.hpp"

#include <iostream>

bool Light::init() {
	// Since we are not going to apply transformation to this screen geometry
	// The coordinates are set to fill the standard openGL window [-1, -1 .. 1, 1]
	// Make the size slightly larger then the screen to crop the boundary.
	static const GLfloat screen_vertex_buffer_data[] = {
		-1.05f, -1.05f, 0.0f,
		1.05f, -1.05f, 0.0f,
		-1.05f,  1.05f, 0.0f,
		-1.05f,  1.05f, 0.0f,
		1.05f, -1.05f, 0.0f,
		1.05f,  1.05f, 0.0f,
	};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertex_buffer_data), screen_vertex_buffer_data, GL_STATIC_DRAW);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("light.vs.glsl"), shader_path("light.fs.glsl")))
		return false;

	return true;
}

// Releases all graphics resources
void Light::destroy() {
	glDeleteBuffers(1, &mesh.vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

// pos is the robot pos
// todo: in the future, light will be slightly above robot,
//  and there might be other lights that are not headlights
void Light::set_position(vec2 pos){
    motion.position = pos;
    light_position = pos;
}

void Light::draw(const mat3& projection, const vec2& camera_shift) {

    // Transformation code, see Rendering and Transformation in the template specification for more info
    // Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
//    transform.begin();
//    transform.translate(camera_shift);
//    transform.translate(motion.position);
//    transform.rotate(motion.radians);
//    transform.end();

    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Set screen_texture sampling to texture unit 0
	// Set clock
	GLuint screen_text_uloc = glGetUniformLocation(effect.program, "screen_texture");
	glUniform1i(screen_text_uloc, 0);

    GLuint light_position_uloc = glGetUniformLocation(effect.program, "light_position");
    // cast light pos to array so we can pass as uniform, for some reason it doesnt like vectors
    vec2 temp_light = add(motion.position , camera_shift);
    temp_light = add(temp_light, vec2{300, -200});
    vec3 light_screen_position = mul(projection, vec3{temp_light.x, temp_light.y, 1});
    float light[] = {light_screen_position.x, light_screen_position.y};
    glUniform2fv(light_position_uloc, 1, light);

	// Draw the screen texture on the quad geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glDisableVertexAttribArray(0);
}
