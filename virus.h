
#pragma once
#ifndef __VIRUS_H__
#define __VIRUS_H__



struct virus_t
{
	vec3	center=vec3(0, 30.0f, 0);		// 2D position for translation
	float	radius=30.0f;		// radius
	float	theta=0.0f;			// rotation angle
	

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update( float time_dif, float curtime, int index);
};



inline void virus_t::update( float initTime, float curtime, int index)
{
	float mytime = curtime - initTime;
	float c = cos(mytime), s = sin(mytime);
	
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
	float	theta = 0.0f;			// rotation angle


	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float time_dif, float curtime, int index, int level);
};

inline void protrusion_t::update(float initTime, float curtime, int index, int level)
{
	float dnum = 10.0f + 5.0f * level;
	float mytime = curtime - initTime;
	float c = cos(mytime + PI * 2.0f / dnum * index), s = sin(mytime + PI * 2.0f / dnum * index);
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


#endif
