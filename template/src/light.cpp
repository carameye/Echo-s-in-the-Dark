#include "light.hpp"
#include "torch.hpp"
#include <math.h>
#include <iostream>
#include <string>

std::map<std::string, Texture> Light::brickmap_textures;

bool Light::init(std::string level) {
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

	if (brickmap_textures.find(level) == brickmap_textures.end()
		|| !brickmap_textures[level].is_valid())
	{
		std::string path = shadow_path;
		path = path.append(level).append("_brickmap.bmp");
		if (!brickmap_textures[level].load_from_file(path.c_str()))
		{
			fprintf(stderr, "Failed to load brickmap texture!");
			return false;
		}
	}

    m_headlight_channel = {1.f, 1.f, 1.f};
	rc.texture = &brickmap_textures[level];

	if (!rc.init_sprite())
		return false;

    return true;
}

// Releases all graphics resources
void Light::destroy() 
{
	for (const auto& it : brickmap_textures)
	{
		Texture t = it.second;
		if (t.id != 0) glDeleteTextures(1, &t.id);
		if (t.depth_render_buffer_id != 0) glDeleteRenderbuffers(1, &t.depth_render_buffer_id);	
	}

	brickmap_textures.clear();

    glDeleteBuffers(1, &mesh.vbo);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

// pos is the robot pos
// todo: in the future, light will be slightly above robot,
// and there might be other lights that are not headlights
void Light::set_position(vec2 pos){
	if (get_direction())
	{
		motion.position = add(pos, { 14.f, -6.f });
	}
	else
	{
		motion.position = add(pos, { -14.f, -6.f });
	}
}

void Light::convert_mouse_pos_to_rad(vec2 coordinates, vec2 centre) {
    float x = coordinates.x - centre.x;
    float y = -coordinates.y + centre.y;
    float radians = atan2(y,x);
    set_rotation(radians);
}

void Light::set_rotation(float radians) {
    motion.radians = radians;
}

float Light::get_radians(){
    return motion.radians;
}

void Light::set_direction(){
    if (cos(motion.radians) > 0){
        motion.radians = -3.14f - motion.radians ;
    }else
        motion.radians = 3.14f - motion.radians ;
}

bool Light::get_direction(){
    return cos(motion.radians) > 0;
}

void Light::set_ambient(float ambient)
{
    this->ambient = ambient;
}

vec3 Light::get_headlight_channel(){
    return m_headlight_channel;
}

void Light::set_red_channel(){
    if  (isRed(m_headlight_channel)) {
        m_headlight_channel = {1.f, 1.f, 1.f};
    } else{
        m_headlight_channel = {1.f, 0.f, 0.f};
    }
}

void Light::set_green_channel(){
    if  (isGreen(m_headlight_channel)){
        m_headlight_channel = {1.f, 1.f, 1.f};
    } else{
        m_headlight_channel = {0.f, 1.f, 0.f};
    }
}

void Light::set_blue_channel(){
    if  (isBlue(m_headlight_channel)){
        m_headlight_channel = {1.f, 1.f, 1.f};
    } else{
        m_headlight_channel = {0.f, 0.f, 1.f};
    }
}

void Light::set_next_light_channel() {
    if  (isWhite(m_headlight_channel)){
        m_headlight_channel = {1.f, 0.f, 0.f};
    } else if  (isRed(m_headlight_channel)) {
        m_headlight_channel = {0.f, 1.f, 0.f};
    } else if (isGreen(m_headlight_channel)) {
        m_headlight_channel = {0.f, 0.f, 1.f};
    }else{
        m_headlight_channel = {1.f, 1.f, 1.f};
    }
}

void Light::set_prev_light_channel() {
    if  (isWhite(m_headlight_channel)){
        m_headlight_channel = {0.f, 0.f, 1.f};
    } else if  (isRed(m_headlight_channel)) {
        m_headlight_channel = {1.f, 1.f, 1.f};
    } else if (isGreen(m_headlight_channel)) {
        m_headlight_channel = {1.f, 0.f, 0.f};
    }else{
        m_headlight_channel = {0.f, 1.f, 0.f};
    }
}

void Light::draw(const mat3& projection, const vec2& camera_shift, const vec2& size, std::vector<Torch*> torches){
    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Set screen_texture sampling to texture unit 0
    // Set clock
    GLuint screen_text_uloc = glGetUniformLocation(effect.program, "screen_texture");
    glUniform1i(screen_text_uloc, 0);

	// Set brick_map sampler uniform
	GLuint brickmap_uloc = glGetUniformLocation(effect.program, "brick_map");
	glUniform1i(brickmap_uloc, 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rc.texture->id);
	glActiveTexture(GL_TEXTURE0);

	// Pass camera position
	GLuint camera_pos_uloc = glGetUniformLocation(effect.program, "camera_pos");
	float cam[] = { camera_shift.x, camera_shift.y };
	glUniform2fv(camera_pos_uloc, 1, cam);

    // pass light position as uniform
    GLuint light_position_uloc = glGetUniformLocation(effect.program, "light_position");
    // cast light pos to array so we can pass as uniform, for some reason it doesnt like vectors
    vec2 light_screen_position = add(motion.position, camera_shift);
    float light[] = {light_screen_position.x, light_screen_position.y};
    glUniform2fv(light_position_uloc, 1, light);

    //pass light angle as uniform
    GLuint light_angle_uloc = glGetUniformLocation(effect.program, "light_angle");
    float angle = motion.radians;
    glUniform1f(light_angle_uloc, angle);

    // pass headlight channel
    GLuint headlight_channel_uloc = glGetUniformLocation(effect.program, "headlight_channel");
    float channel[] = {m_headlight_channel.x, m_headlight_channel.y, m_headlight_channel.z};
    glUniform3fv(headlight_channel_uloc, 1, channel);

	// pass torches size
	int len = (int)torches.size();
	GLuint torches_size_uloc = glGetUniformLocation(effect.program, "torches_size");
	glUniform1i(torches_size_uloc, len);

	// pass all torch positions
	for (int i = 0; i < len && i < 256; i++) {
		std::string s = "torches_position[" + std::to_string(i) + "]";
		GLuint torches_position_uloc = glGetUniformLocation(effect.program, s.c_str());
		float x = -10000.f, y = -10000.f;
		if (i < len)
		{
			x = torches[i]->get_position().x;
			y = torches[i]->get_position().y;
		}
		float torch[] = { x + camera_shift.x, y + camera_shift.y };
		glUniform2fv(torches_position_uloc, 1, torch);
	}

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

bool Light::isWhite(vec3 color) {
    return m_headlight_channel.x == 1.0 && m_headlight_channel.y == 1.0 && m_headlight_channel.z == 1.0;
}

bool Light::isRed(vec3 color) {
    return m_headlight_channel.x == 1.0 && m_headlight_channel.y == 0.0 && m_headlight_channel.z == 0.0;
}

bool Light::isBlue(vec3 color) {
    return m_headlight_channel.x == 0.0 && m_headlight_channel.y == 0.0 && m_headlight_channel.z == 1.0;
}

bool Light::isGreen(vec3 color) {
    return m_headlight_channel.x == 0.0 && m_headlight_channel.y == 1.0 && m_headlight_channel.z == 0.0;
}

