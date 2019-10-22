#include "light.hpp"
#include <math.h>
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
    motion.position = add(pos, { 0.f, -48.f });
}

void Light::set_radians(float radians){
    //  std::cout << motion.radians << std::endl;
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

void Light::clear_torches()
{
    torches.clear();
}

void Light::add_torch(vec2 torch)
{
    torches.push_back(torch);
}

vec3 Light::get_headlight_channel(){
    return headlight_channel;
}

void Light::set_red_channel(){
    if  (headlight_channel.x == 1.0){
        headlight_channel.x = 0.0;
    } else
    headlight_channel.x = 1.0;
}
void Light::set_green_channel(){
    if  (headlight_channel.y == 1.0){
        headlight_channel.y = 0.0;
    } else
        headlight_channel.y = 1.0;
}
void Light::set_blue_channel(){
    if  (headlight_channel.z == 1.0){
        headlight_channel.z = 0.0;
    } else
        headlight_channel.z = 1.0;
}

void Light::draw(const mat3& projection, const vec2& camera_shift, const vec2& size) {
    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Set screen_texture sampling to texture unit 0
    // Set clock
    GLuint screen_text_uloc = glGetUniformLocation(effect.program, "screen_texture");
    glUniform1i(screen_text_uloc, 0);

    // pass light position as uniform
    GLuint light_position_uloc = glGetUniformLocation(effect.program, "light_position");
    // cast light pos to array so we can pass as uniform, for some reason it doesnt like vectors
    vec2 temp_light = add(motion.position, camera_shift);
    temp_light = add(temp_light, vec2{300, -200});
    vec3 light_screen_position = mul(projection, vec3{temp_light.x, temp_light.y, 1});
    float light[] = {light_screen_position.x, light_screen_position.y};
    glUniform2fv(light_position_uloc, 1, light);

    //pass light angle as uniform
    GLuint light_angle_uloc = glGetUniformLocation(effect.program, "light_angle");
    float angle = motion.radians;
    glUniform1f(light_angle_uloc, angle);

    // pass headlight channel
    GLuint headlight_channel_uloc = glGetUniformLocation(effect.program, "headlight_channel");
    float channel[] = {headlight_channel.x, headlight_channel.y, headlight_channel.z};
    glUniform3fv(headlight_channel_uloc, 1, channel);

    if(!(torches.empty())) {
        // pass torches size
        int len = torches.size();
        GLuint torches_size_uloc = glGetUniformLocation(effect.program, "torches_size");
        glUniform1f(torches_size_uloc, len);

        // pass all torch positions
        for (int i = 0; i < 100; i++) {
            char s[50];
            std::sprintf(s,"torches_position[%d]", i );
            GLuint torches_position_uloc = glGetUniformLocation(effect.program, s);
			float x = -10000.f, y = -10000.f;
			if (i < len)
			{
				x = torches[i].x;
				y = torches[i].y;
			}
            float torch[] = { x + camera_shift.x, y + camera_shift.y };
            glUniform2fv(torches_position_uloc, 1, torch);
        }
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
