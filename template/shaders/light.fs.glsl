#version 330

uniform sampler2D screen_texture;
uniform vec2 light_position;

in vec2 uv;

layout(location = 0) out vec4 color;


vec4 illuminate(vec4 in_color, vec2 coord)
{
	vec4 color = in_color;
	float light_radius = 0.3;
	float dist = sqrt(pow((light_position.x - coord.x), 2.0) + pow((light_position.y - coord.y), 2.0));
	float darkness = 0.5;

	if(dist > light_radius){
		color = color * darkness;
	}

	return color;
}

void main()
{
	vec2 coord = uv.xy;// distort(uv);
    vec4 in_color = texture(screen_texture, coord);
	color = illuminate(in_color, coord);
}