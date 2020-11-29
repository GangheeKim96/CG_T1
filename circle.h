
#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__



struct circle_t
{
	vec3	center=vec3(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	float	theta=0.0f;			// rotation angle
	float	self_rotate_vel;
	float   rotate_vel;
	float   distance;
	int		isDwarf = 0;

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update( float time_dif, float curtime, int index);
	void	reset(int index);
};

float radius_arr[11] = { 2.0f, 1.5f, 1.3f, 2.1f, 3.0f,                     
						0.6f, 0.6f,
						1.2f, 0.8f, 2.0f,
						0.6f};

float centerX_arr[11] = { 30.0f, 40.0f, 50.0f, 60.0f, 70.0f,                      
						5.0f, 6.0f,
						80.0f, 90.0f, 100.0f,
						5.0f};

float self_arr[11] = { 0.6f, 1.0f, 0.7f, 1.5f, 0.8f,                       
						3.5f, 3.0f,
						0.6f, 0.7f, 0.7f,
						3.5f};

float rotate_arr[11] = { 1.0f, 1.8f, 1.3f, 1.2f, 1.5f,                 
						3.5f, 4.0f,
						0.8f, 1.4f, 1.1f,
						3.0f};

vec3 dwarfcenter1 = { 70.0f, 0.0f, 0.0f };
vec3 dwarfcenter2 = { 100.0f, 0.0f, 0.0f };

inline std::vector<circle_t> create_circles()
{
	std::vector<circle_t> circles;
	circle_t c;
	
	c = {vec3(0,0,0), 12.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0};
	circles.emplace_back(c);

	for (int i = 1; i < 6; i++) {
		c = { vec3(centerX_arr[i-1],0,0), radius_arr[i-1], 0.0f, self_arr[i-1], rotate_arr[i-1], centerX_arr[i-1], 0 };
		circles.emplace_back(c);
	}
	for (int i = 6; i < 8; i++) {
		c = { vec3(centerX_arr[i - 1],0,0), radius_arr[i - 1], 0.0f, self_arr[i - 1], rotate_arr[i - 1], centerX_arr[i - 1], 1 };
		circles.emplace_back(c);
	}
	for (int i = 8; i < 11; i++) {
		c = { vec3(centerX_arr[i - 1],0,0), radius_arr[i - 1], 0.0f, self_arr[i - 1], rotate_arr[i - 1], centerX_arr[i - 1], 0 };
		circles.emplace_back(c);
	}
	c = { vec3(centerX_arr[10],0,0), radius_arr[10], 0.0f, self_arr[10], rotate_arr[10], centerX_arr[10], 2 };
	circles.emplace_back(c);
	return circles;
}

inline void circle_t::update( float initTime, float curtime, int index)
{
	float mytime = curtime - initTime;
	float c = cos(mytime * self_rotate_vel), s = sin(mytime * self_rotate_vel);
	center.x = distance * cos(mytime * rotate_vel);
	center.y = distance * sin(mytime * rotate_vel);
	

	if (index == 5) {
		dwarfcenter1 = center;
	}

	else if (index == 10) {
		dwarfcenter2 = center;
	}

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};
	
	if (isDwarf == 1) {
		translate_matrix = translate_matrix * mat4::translate(dwarfcenter1);
	}

	else if (isDwarf == 2) {
		translate_matrix = translate_matrix * mat4::translate(dwarfcenter2);
	}

	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

inline void circle_t::reset(int index)
{

	if (!index) center = vec3(0, 0, 0);
	else center = vec3(centerX_arr[index - 1], 0, 0);


	if (index == 5) {
		dwarfcenter1 = center;
	}

	else if (index == 10) {
		dwarfcenter2 = center;
	}

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	if (isDwarf == 1) {
		translate_matrix = translate_matrix * mat4::translate(dwarfcenter1);
	}

	else if (isDwarf == 2) {
		translate_matrix = translate_matrix * mat4::translate(dwarfcenter2);
	}

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}
#endif
