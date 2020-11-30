
#pragma once
#ifndef __VIRUS_H__
#define __VIRUS_H__



struct virus_t
{
	vec3	center=vec3(0, 30.0f, 0);		// 2D position for translation
	float	radius=30.0f;		// radius
	int		state = 0;
	float	hitTime;
	float	prevX = 0;
	float	myX = 0;
	int		vlev = 0;

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update( float curtime, int index);
	void	reset() {
		state = 0;
		hitTime = 0;
		prevX = 0;
		myX = 0;
		vlev = 0;
	};
};



inline void virus_t::update( float curtime, int index)
{
	if (state == 1) {
		if (curtime - hitTime >= 0.1f) {
			state = 0;
		}
	}
	float acc = float(vlev)/5.0f + 1.0f;

	float td = curtime - hitTime;
	td = td * acc;
	float c = cos(prevX + td), s = sin(prevX + td);
	myX = prevX + td;
	
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

	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

struct protrusion_t
{
	vec3	center = vec3(0, 30.0f, 0);		// 2D position for translation
	float	width = 5.0f;
	int		state = 0;
	float	initTime;
	float	prevX = 0;
	float	myX = 0;

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float curtime, int index, int level, int vlev);
	void	reset() {
		state = 0;
		initTime = 0;
		prevX = 0;
		myX = 0;
	};
};

inline void protrusion_t::update(float curtime, int index, int level, int vlev)
{
	float dnum = 10.0f + 5.0f * level;
	float td = curtime - initTime;
	float acc = float(vlev) / 5.0f + 1.0f;
	td = td * acc;
	float c = cos(prevX + (PI * 2.0f / dnum * index) + td), s = sin(prevX + (PI * 2.0f / dnum * index) + td);
	myX = prevX + td;
	center.x = c * 35.0f;
	center.y = 30.0f + s * 35.0f;

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		width, 0, 0, 0,
		0, width, 0, 0,
		0, 0, width, 0,
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

	
	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

struct needle_t
{
	vec3	center = vec3(0, -70.0f, 0);		// 2D position for translation
	float	width = 5.0f;
	int state = 0;
	float shootTime = 0;

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float curtime);
	void	reset() {
		center = vec3(0, -70.0f, 0);
		state = 0;
		shootTime = 0;
	};
};

inline void needle_t::update(float curtime)
{
	float mytime = curtime - shootTime;
	if (state == 1) {
		center.y = -70.0f + mytime*230.0f;
	}
	else if (state == 2) {
		center = { -10000.0f, -10000.0f, -10000.0f };
	}
	
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		width, 0, 0, 0,
		0, width, 0, 0,
		0, 0, width, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		0, -1, 0, 0,
		1, 0, 0, 0,
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

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
