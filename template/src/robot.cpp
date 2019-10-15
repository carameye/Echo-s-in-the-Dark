// Header
#include "robot.hpp"

#include <cmath>

Texture RobotHead::robot_head_texture;
Texture Robot::robot_body_texture;
Texture RobotShoulders::robot_shoulder_texture;

bool RobotHead::init() {
    if (!robot_head_texture.is_valid()) {
        if (!robot_head_texture.load_from_file(textures_path("head.png"))) {
            fprintf(stderr, "Failed to load head texture!");
            return false;
        }
    }

    texture = &robot_head_texture;

    if (!init_sprite())
        return false;

    motion.position = {0.f, 0.f};
    motion.velocity = {0.f, 0.f};
    motion.acceleration = {0.f, 0.f};
    motion.radians = 0.f;

    physics.scale = {1.0f, 1.0f};

    return true;
}

void RobotHead::destroy() {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void RobotHead::update(float ms, vec2 goal) {
    vec2 dist = sub(goal, motion.position);
    set_position(add(get_position(), {0.65f * dist.x, 0.65f * dist.y}));
}

void RobotHead::draw(const mat3 &projection, const vec2 &camera_shift) {
    // Transformation code, see Rendering and Transformation in the template specification for more info
// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    if (!m_face_right)
        transform.scale({-1.f, 1.f});
    transform.end();

    draw_sprite(projection);
}

vec2 RobotHead::get_position() const {
    return motion.position;
}

void RobotHead::set_position(vec2 position) {
    motion.position = position;
}

void RobotHead::set_scaling(vec2 scaling) {
    physics.scale = scaling;
}

void RobotHead::set_direction(bool right) {
    m_face_right = right;
}

bool RobotShoulders::init() {
    if (!robot_shoulder_texture.is_valid()) {
        if (!robot_shoulder_texture.load_from_file(textures_path("body_shoulder.png"))) {
            fprintf(stderr, "Failed to load shoulder texture!");
            return false;
        }
    }

    texture = &robot_shoulder_texture;

    if (!init_sprite())
        return false;

    motion.position = {0.f, 0.f};
    motion.velocity = {0.f, 0.f};
    motion.acceleration = {0.f, 0.f};
    motion.radians = 0.f;

    physics.scale = {1.0f, 1.0f};

    return true;
}

void RobotShoulders::destroy() {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void RobotShoulders::update(float ms, vec2 goal) {
    vec2 dist = sub(goal, motion.position);
    set_position(add(get_position(), {dist.x, dist.y}));
}

void RobotShoulders::draw(const mat3 &projection, const vec2 &camera_shift) {
    // Transformation code, see Rendering and Transformation in the template specification for more info
// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    if (!m_face_right)
        transform.scale({-1.f, 1.f});
    transform.end();

    draw_sprite(projection);
}

vec2 RobotShoulders::get_position() const {
    return motion.position;
}

void RobotShoulders::set_position(vec2 position) {
    motion.position = position;
}

void RobotShoulders::set_scaling(vec2 scaling) {
    physics.scale = scaling;
}

void RobotShoulders::set_direction(bool right) {
    m_face_right = right;
}

bool Robot::init() {
    if (!robot_body_texture.is_valid()) {
        if (!robot_body_texture.load_from_file(textures_path("body_ball.png"))) {
            fprintf(stderr, "Failed to load body texture!");
            return false;
        }
    }

    texture = &robot_body_texture;

    if (!init_sprite())
        return false;

    motion.position = {0.f, 0.f};
    motion.velocity = {0.f, 0.f};
    motion.acceleration = {0.f, 0.f};
    motion.radians = 0.f;

    physics.scale = {brick_size / texture->width, brick_size / texture->height};
    bool valid = m_head.init() && m_shoulders.init();
    m_head.set_scaling(physics.scale);
    m_shoulders.set_scaling(physics.scale);

    return valid;
}

// Releases all graphics resources
void Robot::destroy() {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Robot::update(float ms) {
    // TODO: handle  key strokes from world
    if (m_grounded && std::abs(motion.velocity.x) > TOLERANCE)
        motion.radians += motion.velocity.x / 1000;

    m_grounded = false;
    m_head.update(ms, add(motion.position, {0.f, -48.f}));
    m_shoulders.update(ms, add(motion.position, {0.f, 0.f}));

    if (motion.acceleration.x != 0.f) {
        m_head.set_direction(motion.acceleration.x > 0.f);
        m_shoulders.set_direction(motion.acceleration.x > 0.f);
    }
}

void Robot::draw(const mat3 &projection, const vec2 &camera_shift) {
    // Transformation code, see Rendering and Transformation in the template specification for more info
    // Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    draw_sprite(projection);

    m_shoulders.draw(projection, camera_shift);
    m_head.draw(projection, camera_shift);
}

vec2 Robot::get_position() const {
    return motion.position;
}

vec2 Robot::get_velocity() const {
    return motion.velocity;
}

vec2 Robot::get_acceleration() const {
    return motion.acceleration;
}

void Robot::set_position(vec2 position) {
    motion.position = position;
}

void Robot::set_velocity(vec2 velocity) {
    motion.velocity = velocity;
}

void Robot::set_acceleration(vec2 acceleration) {
    motion.acceleration = acceleration;
}

void Robot::set_grounded() {
    m_grounded = true;
}

void Robot::set_head_position(vec2 position) {
    m_head.set_position(position);
}

void Robot::set_shoulder_position(vec2 position) {
    m_shoulders.set_position(position);
}

Hitbox Robot::get_hitbox(vec2 translation) const {
    std::vector<Circle> circles(1);

    vec2 position = motion.position;

    position.x += translation.x;
    position.y += translation.y;

    int radius = brick_size / 2;
    Circle circle(position, radius);
    circles[0] = circle;

    Hitbox hitbox(circles, {});
    // TODO: figure out why this line is not working
    // hitbox.translate(translation);
    return hitbox;
}
