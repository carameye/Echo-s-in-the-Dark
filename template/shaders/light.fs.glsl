#version 330

uniform sampler2D screen_texture;
uniform vec2 light_position;

in vec2 uv;

layout(location = 0) out vec4 color;


vec4 illuminate(vec4 in_color, vec2 coord)
{
	vec4 color = in_color;
	float light_radius = 100;
	vec2 pos = vec2(600 + (light_position.x*1200 - 600) / 2.1, 400 + (light_position.y*800 - 400) / 2.1);
	float dist = sqrt(pow((pos.x - coord.x*1200), 2.0) + pow((pos.y - coord.y*800), 2.0));
	float darkness = 1.4;

	color = color * (1- darkness * dist/1000);
	return color;
}

void main()
{
	vec2 coord = uv.xy;
    vec4 in_color = texture(screen_texture, coord);
	color = illuminate(in_color, coord);
}