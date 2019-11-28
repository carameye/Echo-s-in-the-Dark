#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;

uniform vec3 headlight_channel;
uniform vec3 component_colour;
uniform int component_can_be_hidden;
uniform int component_is_invisible;

// Output color
layout(location = 0) out  vec4 color;

bool colour_equals(vec3 col1, vec3 col2) {
	return col1.x == col2.x && col1.y == col2.y && col1.z == col2.z;
}

void main()
{
	if (component_is_invisible == 1) {
		vec4 no_alpha;
		no_alpha.x = 0.f;
		no_alpha.y = 0.f;
		no_alpha.z =  0.f;
		no_alpha.w = 0.f;
		color = no_alpha * texture(sampler0, vec2(texcoord.x, texcoord.y));
	}else if (component_can_be_hidden == 1) {
		if (colour_equals(headlight_channel, component_colour)){
			color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
		} else {
			vec4 low_alpha;
			low_alpha.x = component_colour.x;
			low_alpha.y = component_colour.y;
			low_alpha.z =  component_colour.z;
			low_alpha.w = 0.1;
			color = low_alpha * texture(sampler0, vec2(texcoord.x, texcoord.y));
		}
	} else {
		color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
	}
}
