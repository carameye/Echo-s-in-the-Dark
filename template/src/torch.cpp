#include "torch.hpp"

Texture Torch::torch_texture;
RenderComponent Torch::rc;

bool Torch::init(int id)
{
    m_id = id;

    if (!torch_texture.is_valid())
    {
        if (!torch_texture.load_from_file(textures_path("light.png")))
        {
            fprintf(stderr, "Failed to load torch texture!");
            return false;
        }

        rc.texture = &torch_texture;

        if (!rc.init_sprite())
            return false;
    }

    mc.position = { 0.f, 0.f };
    mc.velocity = { 0.f, 0.f };
    mc.acceleration = { 0.f , 0.f };
    mc.radians = 0.f;
    mc.physics.scale = { 1.5f, 1.5f };

    s_render_components[id] = &rc;
    s_motion_components[id] = &mc;

    return true;
}

void Torch::update(float ms)
{
    // probably don't really need much here...
}

vec2 Torch::get_position()const
{
    return mc.position;
}

void Torch::set_position(vec2 position)
{
    mc.position = position;
	mc.position.y += 40;
}
