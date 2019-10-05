#version 330

uniform sampler2D screen_texture;
uniform vec2 light_position;

in vec2 uv;

layout(location = 0) out vec4 color;


vec4 illuminate(vec4 in_color, vec2 coord)
{
	vec4 color = in_color;
	float light_radius = 100;
	//todo: light pos is not offset correctly rn, so we just hardcode
	//float dist = sqrt(pow((light_position.x - coord.x), 2.0) + pow((light_position.y - coord.y), 2.0));
	float dist = sqrt(pow((600 - coord.x*1200), 2.0) + pow((400 - coord.y *800), 2.0));
	float darkness = 1.4;

	color = color *  (1- darkness * dist/1000);
	return color ;
}

void main()
{
	vec2 coord = uv.xy;
    vec4 in_color = texture(screen_texture, coord);
	color = illuminate(in_color, coord);
}