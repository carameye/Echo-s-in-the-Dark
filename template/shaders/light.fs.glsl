#version 330

uniform sampler2D screen_texture;
uniform vec2 light_position;
uniform vec2 torches_position_x;
uniform vec2 torches_position_y;

uniform float light_angle;

in vec2 uv;

layout(location = 0) out vec4 color;


float illuminate_robot(vec4 in_color, vec2 coord)
{
	vec4 color = in_color;
	float light_radius = 100;
	vec2 robot_pos = vec2(640, 400); // robot is always constant
	//vec2 pos = vec2(600 + (light_position.x*1200 - 600) / 2.1, 400 + (light_position.y*800 - 400) / 2.1);
	float dist = sqrt(pow((robot_pos.x - coord.x*1200), 2.0) + pow((robot_pos.y - coord.y*800), 2.0));
	float darkness = 1.4;

	return (1- darkness * dist/400);
}

float illuminate_torches(vec4 in_color, vec2 coord, vec2 torchcoord)
{
	vec4 color = in_color;
	float light_radius = 100;
	vec2 pos = torchcoord;
	//vec2 pos = vec2(torches_position_x.x, torches_position_y.x);
	//vec2 pos = vec2(600, 400);
	float dist = sqrt(pow((pos.x - coord.x*1200), 2.0) + pow((pos.y - coord.y*800), 2.0));
	float darkness = 1.5;
	return (1- darkness * dist/600);
}

float headlight(vec4 in_color, vec2 coord){
	vec4 color = in_color;
	// if the cord inside the area covered by headlight?
	// hardcoded light info
	vec2 light_pos = vec2(600 + (light_position.x*1200 - 600) / 2.1, 400 + (light_position.y*800 - 400) / 2.1);
	//vec2 cone_dir = vec2(1, 1);
	vec2 cone_dir = vec2(cos(light_angle), sin(light_angle));
	// TODO: cone_dir comes from cone angle
	cone_dir = normalize(cone_dir);

	// coord in pixels
	vec2 coord_px = vec2(coord.x * 1200, coord.y*800);
	// make a line from light pos to coord
	vec2 line =  coord_px - light_pos;
	line = normalize(line);
	float slope_line = line.y / line.x;
	//float angle_line = atan(slope_line);
	float dot_pr = dot(line, cone_dir);
	float darkness = 0.6;

	float dist = sqrt(pow((light_pos.x - coord_px.x), 2.0) + pow((light_pos.y -coord_px.y), 2.0));

	if (dot_pr < 0){
		return 0;
	}
	if (dot_pr > 0.8){
		return (1 - dist/600) * pow(dot_pr,5);
	}
	return 0;//darkness*pow(dot_pr,5);//*(1- darkness * dist/600);

}

void main()
{
	vec2 coord = uv.xy;
	vec4 in_color = texture(screen_texture, coord);
	// illuminate for torches first
	// iterate over torches_pos_x and torches_pos_y
	float illum_torches1 = clamp(illuminate_torches(in_color, coord, vec2(torches_position_x.x, torches_position_y.x)), 0 , 1);
	float illum_torches2 = clamp(illuminate_torches(in_color, coord, vec2(torches_position_x.y, torches_position_y.y)), 0 , 1);
	float illum_torches = clamp(illum_torches1+ illum_torches2, 0, 1);

	float illum_robot = clamp(illuminate_robot(in_color, coord), 0, 1);
	//float illum_torches = clamp(illuminate_torches(in_color, coord), 0 , 1);
	//float illum = clamp(illuminate(in_color, coord), 0, 1);
	float hl = clamp(headlight(in_color, coord), 0, 1);
	float sum = clamp(hl + illum_torches + illum_robot, 0, 1);

	color = in_color *sum;//* illum_torches * hl;
	//	color = in_color * illum;
}
