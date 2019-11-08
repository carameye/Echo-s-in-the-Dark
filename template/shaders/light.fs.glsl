#version 330

uniform sampler2D screen_texture;
uniform sampler2D brick_map;

uniform vec3 headlight_channel;
uniform vec2 camera_pos;
uniform vec2 light_position;
uniform vec2 torches_position[256];
uniform int torches_size;
uniform float light_angle;

in vec2 uv;
layout(location = 0) out vec4 color;

vec2 screen_size;
vec2 shadow_size;
vec2 light_pos;
vec4 in_color;

vec2 to_pixels(vec2 p)
{
    float x = screen_size.x / 2 + (p.x * screen_size.x - screen_size.x / 2);
    float y = screen_size.y / 2 + (p.y * screen_size.y - screen_size.y / 2);
	return vec2(x, y);
}

float dist(vec2 a, vec2 b)
{
	vec2 d = a - b;
	return sqrt(pow(d.x, 2) + pow(d.y, 2));
}

float illuminate_robot(vec2 coord)
{
	float dist = dist(light_pos, vec2(coord.x * screen_size.x, coord.y * screen_size.y));
	float darkness = 1.5;
	return sqrt(1 - darkness * dist / 300) / 1.2;
}

float illuminate_torches(vec2 coord, vec2 pos)
{
	coord.y = 1 - coord.y;
	float dist = dist(pos, vec2(coord.x * screen_size.x, coord.y * screen_size.y));
	float darkness = 1.5;
	return sqrt(1 - darkness * dist / 400);
}

float headlight(vec2 coord) 
{
	vec2 cone_dir = vec2(cos(light_angle), sin(light_angle));
	cone_dir = normalize(cone_dir);

	vec2 coord_px = vec2(coord.x * screen_size.x, coord.y * screen_size.y);
	vec2 line =  normalize(coord_px - light_pos);

    float angle_diff = acos(dot(cone_dir, line));

    float max_diff = 3.1415 / 6;
    if (abs(angle_diff) > max_diff) {
        return 0;
    }

    return sqrt(1 - angle_diff / max_diff);
}

float get_light_at_pixel(vec2 pixel)
{
	vec2 brick_coord = vec2(screen_size.x - pixel.x, screen_size.y - pixel.y) + camera_pos - screen_size - vec2(32, 32);
	brick_coord = vec2(1 - brick_coord.x / shadow_size.x, 1 - brick_coord.y / shadow_size.y);
	return texture(brick_map, brick_coord).x;
}

float find_light_space(vec2 p1, vec2 p2)
{
    vec2 p = vec2(p1.x, p1.y);
    vec2 d = vec2(p2.x - p1.x, p2.y - p1.y);

    float hit_count = 0;
    float max_hits = 20;
    float step_size = 4;

    d = normalize(d);

    while (dist(p1, p) < dist(p1, p2))
    {
        if (get_light_at_pixel(p) == 0)
        {
            hit_count = hit_count + 1;
        }

        if (hit_count > max_hits)
        {
        	return 0;
        }

        p = p + step_size * d;
    }

    return 1 - hit_count / max_hits;
}

float find_light_brick(vec2 p1, vec2 p2)
{
    vec2 p = vec2(p1.x, p1.y);
    vec2 d = vec2(p2.x - p1.x, p2.y - p1.y);

    float diff_x = p.x - camera_pos.x + 32;
    diff_x = mod(diff_x, 64);
    if (diff_x < 32)
    {
        diff_x = -diff_x - 1;
    }
    else
    {
        diff_x = 64 - diff_x + 1;
    }

    float diff_y = p.y - camera_pos.y + 32;
    diff_y = mod(diff_y, 64);
    if (diff_y < 32)
    {
        diff_y = -diff_y - 1;
    }
    else
    {
        diff_y = 64 - diff_y + 1;
    }

    float p_x = p.x + diff_x;
    float p_y = p.y + diff_y;

    float c_x = 0;
    if (abs(diff_x) <= 32)
    {
        c_x = illuminate_torches(vec2(p_x / screen_size.x, p.y / screen_size.y), p2);
        c_x = c_x * find_light_space(vec2(p_x, p.y), p2) * get_light_at_pixel(vec2(p_x, p.y));
        c_x = c_x * sqrt(1024 - pow(diff_x, 2)) / 32;
    }

    float c_y = 0;
    if (abs(diff_y) <= 32)
    {
        c_y = illuminate_torches(vec2(p.x / screen_size.x, p_y / screen_size.y), p2);
        c_y = c_y * find_light_space(vec2(p.x, p_y), p2) * get_light_at_pixel(vec2(p.x, p_y));
        c_y = c_y * sqrt(1024 - pow(diff_y, 2)) / 32;
    }

    float c = pow(c_x, 2) + pow(c_y, 2);
    if (abs(diff_x) <= 32 && abs(diff_y) <= 32 && c == 0)
    {
        c = illuminate_torches(vec2(p_x / screen_size.x, p_y / screen_size.y), p2);
        c = c * find_light_space(vec2(p_x, p_y), p2) * get_light_at_pixel(vec2(p_x, p_y));
        c = c * sqrt(1024 - (pow(diff_x, 2) + pow(diff_y, 2)) / 2) / 32;
        return pow(c, 2);
    }

    return sqrt(c);
}

float find_light(vec2 p1, vec2 p2)
{
    if (p1.x == p2.x && p1.y == p2.y)
    {
        return 1;
    }

    if (get_light_at_pixel(p1) != 0)
    {
        return find_light_space(p1, p2);
    }
    else
    {
        return find_light_brick(p1, p2);
    }
}

void main()
{
	screen_size = textureSize(screen_texture, 0);
	shadow_size = textureSize(brick_map, 0);
    light_pos = light_position;
    light_pos.y = 800 - light_pos.y;

	vec2 coord = uv.xy;
	in_color = texture(screen_texture, coord);
	vec4 headlight_channels = vec4(headlight_channel, 1.0);

    vec2 pos = vec2(coord.x * screen_size.x, (1 - coord.y) * screen_size.y);

    vec2 w_p = pos - camera_pos + vec2(32, 32);
    if (w_p.x < 0 || w_p.x > shadow_size.x || w_p.y < 0 || w_p.y > shadow_size.y)
    {
        color = vec4(0, 0, 0, 0);
        return;
    }

	// illuminate for torches first
	float illum_torch_sum = 0;
	for (int i = 0; i < torches_size; i++) {
        if (dist(pos, torches_position[i]) > 384) {
            continue;
        }
        float t_light = find_light(pos, torches_position[i]);
		float illum_torch = illuminate_torches(coord, torches_position[i]) * t_light;
		illum_torch_sum = max(illum_torch_sum, illum_torch);
	}

    vec2 temp_l = vec2(light_pos.x, screen_size.y - light_pos.y);
    float hl_light = find_light(pos, temp_l);

	float illum_robot = clamp(illuminate_robot(coord), 0, 1) * hl_light;
	float hl = clamp(headlight(coord), 0, 0.8) * hl_light;

	float sum = clamp(hl + illum_torch_sum + illum_robot, 0, 0.9);

	if (headlight_channels.x == 0 && headlight_channels.y == 0 && headlight_channels.z == 0) {
		color = in_color * sum;
	} else {
		color = mix( in_color,headlight_channels, hl) * sum;
	}

}
