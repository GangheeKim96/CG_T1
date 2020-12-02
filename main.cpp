#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "virus.h"			// circle class definition
#include "trackball.h"
#include "irrKlang\irrKlang.h"
#pragma comment(lib, "irrKlang.lib")
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//*************************************
// global constants
static const char*	window_name = "KGH SHH CG T1";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const char*	sound_btn_path = "../bin/sounds/btn.wav";
static const char*	sound_shoot_path = "../bin/sounds/shoot.wav";
static const char*	sound_hit_path = "../bin/sounds/hit.wav";
static const char*	sound_success_path = "../bin/sounds/success.wav";
static const char*	sound_fail_path = "../bin/sounds/fail.wav";
static const char*	sound_bgm_path = "../bin/sounds/bgm.mp3";
static const char*	sound_miss_path = "../bin/sounds/miss.mp3"; //CC0 1.0 Universal made by qubodup https://freesound.org/people/qubodup/packs/12143/

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	a = 1.0f;

//*******************************************************************
// irrKlang objects
irrklang::ISoundEngine* engine;
irrklang::ISoundSource* btn_src = nullptr;
irrklang::ISoundSource* shoot_src = nullptr;
irrklang::ISoundSource* hit_src = nullptr;
irrklang::ISoundSource* success_src = nullptr;
irrklang::ISoundSource* fail_src = nullptr;
irrklang::ISoundSource* bgm_src = nullptr;
irrklang::ISoundSource* miss_src = nullptr;

//*******************************************************************
// forward declarations for freetype text
bool init_text();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_vertices;	// host-side vertices

//*************************************

struct camera {
	vec3	eye = vec3(0, 0, 1000);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 1, 0);
	mat4	view_matrix = mat4::look_at(eye, at, up);
	float fovy = PI / 6.0f;
	float aspect_ratio;
	float dnear = 1.0f;
	float dfar = 1000.0f;
	mat4	projection_matrix;

	void update(vec3 newEye) {
		eye = newEye;
		view_matrix = mat4::look_at(newEye, at, up);
	}
};

struct light_t
{
	vec4	position = vec4(0.0f, 50.0f, 50.0f, 1.0f);
	vec4	ambient = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vec4	diffuse = vec4(0.6f, 0.6f, 0.6f, 1.0f);
	vec4	specular = vec4(0.2f, 0.2f, 0.2f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vec4	diffuse = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	vec4	specular = vec4(2.0f, 2.0f, 2.0f, 1.0f);
	float	shininess = 200.0f;
};


camera cam;
trackball tb;
light_t		light;
material_t	material;
float curtime = 0.0f;
int game_state = 0;
int game_level = 0;
int die = 0;
virus_t virus;
protrusion_t prots[20];
needle_t needles[23];
sphere_t spheres[3];
int shootIndex = 0;

static const char* texture_path_Title = "../bin/textures/title.jpg";
static const char* texture_path_Help = "../bin/textures/help.jpg";
static const char* texture_path_Cleared = "../bin/textures/cleared.jpg";
static const char* texture_path_Failed = "../bin/textures/failed.jpg";
static const char* texture_path_Virus = "../bin/textures/virus.jpg";
static const char* texture_path_Needle = "../bin/textures/needle.jpg";
static const char* texture_path_Prot = "../bin/textures/protrusion.jpg";
GLuint texture_Title = 0;
GLuint texture_Help = 0;
GLuint texture_Cleared = 0;
GLuint texture_Failed = 0;
GLuint texture_Virus = 0;
GLuint texture_Needle = 0;
GLuint texture_Prot = 0;

void update()
{
	//printf("GITHUB PUSH TEST!!!\n");
	// update global simulation parameter
	curtime = float(glfwGetTime());

	// text alpha value
	if (a > 0) a -= (float)glfwGetTimerFrequency() * 0.0000000001f;

	// tricky aspect correction matrix for non-square window
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(
		cam.fovy, cam.aspect_ratio, cam.dnear, cam.dfar
	);

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");       if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix"); if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	
	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);
	
}

void render()
{
	// render texts
	//float dpi_scale = cg_get_dpi_scale();
	//render_text("START!", 0, 100, 2.5f, vec4(1, 1, 1, a), dpi_scale);

	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_Title);
	glUniform1i(glGetUniformLocation(program, "TEX_TITLE"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_Help);
	glUniform1i(glGetUniformLocation(program, "TEX_HELP"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_Cleared);
	glUniform1i(glGetUniformLocation(program, "TEX_CLEARED"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture_Failed);
	glUniform1i(glGetUniformLocation(program, "TEX_FAILED"), 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture_Virus);
	glUniform1i(glGetUniformLocation(program, "TEX_VIRUS"), 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture_Needle);
	glUniform1i(glGetUniformLocation(program, "TEX_NEEDLE"), 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, texture_Prot);
	glUniform1i(glGetUniformLocation(program, "TEX_PROT"), 6);

	// bind vertex array object
	glBindVertexArray( vertex_array );

	mat4 temp = { 1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1 };
	GLint uloc;

	
	if (game_state == 2) {

		//클리어 확인함
		if (virus.vlev == 10 + game_level * 5) {
			engine->play2D(success_src, false);
			game_state = 3;
			printf("111game_state = %d\n", game_state);
			cam.update(vec3(0, 0, 800.0f));
		}

		//실패 확인함
		else if (die == 3) {
			engine->play2D(fail_src, false);
			game_state = 4;
			printf("222game_state = %d\n", game_state);
			cam.update(vec3(0, 0, 700.0f));
		}

		if (game_state == 2){

			//맞춘게 있는지 확인함
			for (int i = 0; i < (10 + game_level * 5) + 3; i++) {
				if (needles[i].state == 1) {
					float nx = needles[i].center.x;
					float ny = needles[i].center.y;
					for (int j = 0; j < (10 + game_level * 5); j++) {
						float px = prots[j].center.x;
						float py = prots[j].center.y;
						float dx = px - nx;
						float dy = py - ny;
						if (dx <= 4.0f && dx >= -4.0f &&
							dy <= 8.0f && dy >= -8.0f && needles[i].state == 1 && prots[j].state == 0) {
							virus.state = 1;
							virus.hitTime = curtime;
							virus.vlev++;
							virus.prevX = virus.myX;
							engine->play2D(hit_src, false);
							for (int k = 0; k < 10 + game_level * 5; k++) {
								prots[k].initTime = curtime;
								prots[k].prevX = prots[k].myX;
							}
							needles[i].state = 2;
							prots[j].state = 1;
						}
					}
					if (ny >= 25.0f) {
						if (needles[i].state == 1) {
							spheres[die].state = 1;
							die++;
						}
						if(die!=3)	engine->play2D(miss_src, false);
						needles[i].state = 2;
					}
				}
			}

			virus.update(curtime, 0);
			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, virus.model_matrix);
			glUniform1i(glGetUniformLocation(program, "drawing_obj"), 0);
			glUniform1i(glGetUniformLocation(program, "obj_state"), virus.state);
			glDrawElements(GL_TRIANGLES, 72 * 3, GL_UNSIGNED_INT, nullptr);

			for (int i = 0; i < (10 + game_level * 5); i++) {
				prots[i].update(curtime, i, game_level, virus.vlev);
				uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, prots[i].model_matrix);
				glUniform1i(glGetUniformLocation(program, "drawing_obj"), 1);
				glUniform1i(glGetUniformLocation(program, "obj_state"), prots[i].state);
				glDrawArrays(GL_TRIANGLES, 104, 6); // (topology, start offset, no. vertices)
			}

			for (int i = 0; i < (10 + game_level * 5) + 3; i++) {
				needles[i].update(curtime);
				uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, needles[i].model_matrix);
				glUniform1i(glGetUniformLocation(program, "drawing_obj"), 2);
				glUniform1i(glGetUniformLocation(program, "obj_state"), needles[i].state);
				glDrawArrays(GL_TRIANGLES, 80, 6); // (topology, start offset, no. vertices)
			}

			for (int i = 0; i < 3; i++) {
				spheres[i].update();
				uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, spheres[i].model_matrix);
				glUniform1i(glGetUniformLocation(program, "drawing_obj"), 3);
				glUniform1i(glGetUniformLocation(program, "obj_state"), spheres[i].state);
				//glDrawArrays(GL_TRIANGLES, 110, 72 * 36 * 3); // (topology, start offset, no. vertices)
				glDrawElements(GL_TRIANGLES, 72 * 36 * 6, GL_UNSIGNED_INT, (void*)(sizeof(uint)*(72*3+36)));
			}
		}
	}

	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, temp);

	
		glUniform1i(glGetUniformLocation(program, "drawing_obj"), 100);
		glDrawArrays(GL_TRIANGLES, 74, 6); // (topology, start offset, no. vertices)
	
	
		glUniform1i(glGetUniformLocation(program, "drawing_obj"), 101);
		glDrawArrays(GL_TRIANGLES, 86, 6); // (topology, start offset, no. vertices)
	
	
		glUniform1i(glGetUniformLocation(program, "drawing_obj"), 102);
		glDrawArrays(GL_TRIANGLES, 92, 6); // (topology, start offset, no. vertices)
	
	
		glUniform1i(glGetUniformLocation(program, "drawing_obj"), 103);
		glDrawArrays(GL_TRIANGLES, 98, 6); // (topology, start offset, no. vertices)
	
	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
	
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'r' to rotate or stop\n" );
	

	printf( "\n" );
}

std::vector<vertex> create_vertices( void )
{
	std::vector<vertex> v = { { vec3(0), vec3(0,0,1.0f), vec2(0.5f) } }; // origin
	for (uint k = 0; k <= 72; k++)
	{
		float t = PI * 2.0f * k / float(72), c = cos(t), s = sin(t);
		v.push_back({ vec3(c,s,0), vec3(0,0,1.0f), vec2(c,s) * 0.5f + 0.5f });
	}
	//여기까지 74개임

	vertex title[4];
	title[0].pos = vec3(-22.0f, -12.0f, 950.0f);	title[0].tex = vec2(0.0f, 0.0f);
	title[1].pos = vec3(+22.0f, -12.0f, 950.0f);	title[1].tex = vec2(1.0f, 0.0f);
	title[2].pos = vec3(+22.0f, +12.0f, 950.0f);	title[2].tex = vec2(1.0f, 1.0f);
	title[3].pos = vec3(-22.0f, +12.0f, 950.0f);	title[3].tex = vec2(0.0f, 1.0f);
	v.push_back(title[0]); v.push_back(title[1]); v.push_back(title[2]);
	v.push_back(title[0]); v.push_back(title[2]); v.push_back(title[3]);
	//여기까지 80개임

	vertex needle[4];
	needle[0].pos = vec3(-1.0f, -0.5f, 0.0f);	needle[0].tex = vec2(0.0f, 0.0f);	needle[0].norm = vec3(0, 0, 1.0f);
	needle[1].pos = vec3(+1.0f, -0.5f, 0.0f);	needle[1].tex = vec2(1.0f, 0.0f); needle[1].norm = vec3(0, 0, 1.0f);
	needle[2].pos = vec3(+1.0f, +0.5f, 0.0f);	needle[2].tex = vec2(1.0f, 1.0f); needle[2].norm = vec3(0, 0, 1.0f);
	needle[3].pos = vec3(-1.0f, +0.5f, 0.0f);	needle[3].tex = vec2(0.0f, 1.0f); needle[3].norm = vec3(0, 0, 1.0f);
	v.push_back(needle[0]); v.push_back(needle[1]); v.push_back(needle[2]);
	v.push_back(needle[0]); v.push_back(needle[2]); v.push_back(needle[3]);
	//여기까지 86개임

	vertex help[4];
	help[0].pos = vec3(-22.0f, -12.0f, 850.0f);	help[0].tex = vec2(0.0f, 0.0f);
	help[1].pos = vec3(+22.0f, -12.0f, 850.0f);	help[1].tex = vec2(1.0f, 0.0f);
	help[2].pos = vec3(+22.0f, +12.0f, 850.0f);	help[2].tex = vec2(1.0f, 1.0f);
	help[3].pos = vec3(-22.0f, +12.0f, 850.0f);	help[3].tex = vec2(0.0f, 1.0f);
	v.push_back(help[0]); v.push_back(help[1]); v.push_back(help[2]);
	v.push_back(help[0]); v.push_back(help[2]); v.push_back(help[3]);

	vertex cleared[4];
	cleared[0].pos = vec3(-22.0f, -12.0f, 750.0f);	cleared[0].tex = vec2(0.0f, 0.0f);
	cleared[1].pos = vec3(+22.0f, -12.0f, 750.0f);	cleared[1].tex = vec2(1.0f, 0.0f);
	cleared[2].pos = vec3(+22.0f, +12.0f, 750.0f);	cleared[2].tex = vec2(1.0f, 1.0f);
	cleared[3].pos = vec3(-22.0f, +12.0f, 750.0f);	cleared[3].tex = vec2(0.0f, 1.0f);
	v.push_back(cleared[0]); v.push_back(cleared[1]); v.push_back(cleared[2]);
	v.push_back(cleared[0]); v.push_back(cleared[2]); v.push_back(cleared[3]);

	vertex failed[4];
	failed[0].pos = vec3(-22.0f, -12.0f, 650.0f);	failed[0].tex = vec2(0.0f, 0.0f);
	failed[1].pos = vec3(+22.0f, -12.0f, 650.0f);	failed[1].tex = vec2(1.0f, 0.0f);
	failed[2].pos = vec3(+22.0f, +12.0f, 650.0f);	failed[2].tex = vec2(1.0f, 1.0f);
	failed[3].pos = vec3(-22.0f, +12.0f, 650.0f);	failed[3].tex = vec2(0.0f, 1.0f);
	v.push_back(failed[0]); v.push_back(failed[1]); v.push_back(failed[2]);
	v.push_back(failed[0]); v.push_back(failed[2]); v.push_back(failed[3]);

	vertex prot[4];
	prot[0].pos = vec3(-1.0f, -0.5f, 0.0f);	prot[0].tex = vec2(1.0f, 0.0f);	prot[0].norm = vec3(0, 0, 1.0f);
	prot[1].pos = vec3(+1.0f, -0.5f, 0.0f);	prot[1].tex = vec2(0.0f, 0.0f); prot[1].norm = vec3(0, 0, 1.0f);
	prot[2].pos = vec3(+1.0f, +0.5f, 0.0f);	prot[2].tex = vec2(0.0f, 1.0f); prot[2].norm = vec3(0, 0, 1.0f);
	prot[3].pos = vec3(-1.0f, +0.5f, 0.0f);	prot[3].tex = vec2(1.0f, 1.0f); prot[3].norm = vec3(0, 0, 1.0f);
	v.push_back(prot[0]); v.push_back(prot[1]); v.push_back(prot[2]);
	v.push_back(prot[0]); v.push_back(prot[2]); v.push_back(prot[3]);
	//여기까지 110개임

	for (int i = 0; i <= 36; i++)
	{
		for (int j = 0; j <= 72; j++) {
			float theta = PI * i / float(36);
			float cc = cos(theta);
			float ss = sin(theta);
			float alpha = PI * 2.0f * j / float(72);
			float c = cos(alpha);
			float s = sin(alpha);
			v.push_back({ vec3(ss * c,ss * s,cc),
				vec3(ss * c,ss * s,cc),
				vec2(alpha / (2 * PI),1 - theta / PI) });

		}

	}

	return v;
	
}



void update_vertex_buffer( const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffers
	
	std::vector<uint> indices;
	
	for (uint k = 0; k < 72; k++)
	{
		indices.push_back(0);	// the origin
		indices.push_back(k + 1);
		indices.push_back(k + 2);
	}

	int cnt = 74;
	for (int i = 0; i < 36; i++) {
		indices.push_back(cnt + i);
	}
	
	cnt = 110;

	for (int i = 0; i < 36; i++)
	{
		for (int j = 0; j < 72; j++) {
			indices.push_back(73 * i + j + cnt);
			indices.push_back(73 * (i + 1) + j + cnt);
			indices.push_back(73 * i + j + 1 + cnt);
			indices.push_back(73 * i + j + 1 + cnt);
			indices.push_back(73 * (i + 1) + j + cnt);
			indices.push_back(73 * (i + 1) + j + 1 + cnt);
		}

	}

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers( 1, &index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	
	

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	vertex_array = cg_create_vertex_array( vertex_buffer, index_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return; }
	
}



void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );

		else if (key == GLFW_KEY_F1) {
			if(game_state!=2)	engine->play2D(btn_src, false);
			if (game_state == 0) {
				game_state = 1;
				printf("333game_state = %d\n", game_state);
				cam.update(vec3(0.0f, 0.0f, 900.0f));
				//printf("cam.eye = (%f, %f, %f)\n", cam.eye[0], cam.eye[1], cam.eye[2]);
			}
			else if (game_state == 1) {
				game_state = 0;
				printf("444game_state = %d\n", game_state);
				cam.update(vec3(0.0f, 0.0f, 1000.0f));
				//printf("cam.eye = (%f, %f, %f)\n", cam.eye[0], cam.eye[1], cam.eye[2]);
			}
		}
		
		else if (key == GLFW_KEY_R) {
			engine->play2D(btn_src, false);
			game_state = 0;
			printf("555game_state = %d\n", game_state);
			virus.reset();
			for (int i = 0; i < 20; i++) {
				prots[i].reset();
			}
			for (int i = 0; i < 23; i++) {
				needles[i].reset();
			}
			for (int i = 0; i < 3; i++) {
				spheres[i].reset();
			}
			die = 0;
			shootIndex = 0;
			cam.update(vec3(0, 0, 1000.0f));
		}

		else if (key == GLFW_KEY_A && game_state == 2) {
			engine->play2D(shoot_src, false);
			needles[shootIndex].state = 1;
			needles[shootIndex].shootTime = curtime;
			shootIndex++;
		}

		else if (key == GLFW_KEY_SPACE && game_state == 2) {
			engine->play2D(btn_src, false);
			cam.update(vec3(0, 0, 300.0f));
		}

		else if ((key == GLFW_KEY_0 || key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_KP_0
					|| key == GLFW_KEY_KP_1 || key == GLFW_KEY_KP_2) && game_state == 0)
		{
			engine->play2D(btn_src, false);
			a = 1.0f;
			if (key < GLFW_KEY_KP_0) {
				game_level = key - GLFW_KEY_0;
			}
			else {
				game_level = key - GLFW_KEY_KP_0;
			}
			game_state = 2;
			printf("666game_state = %d\n", game_state);
			virus.hitTime = curtime;
			for (int i = 0; i < 10 + game_level * 5; i++) {
				prots[i].initTime = curtime;
			}
			cam.update(vec3(0, 0, 300.0f));
		}
	}
	
	
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
	vec2 npos = cursor_to_ndc(pos, window_size);
	printf("(%f, %f) clicked.\n", npos.x, npos.y);

	/*if (button == GLFW_MOUSE_BUTTON_LEFT && game_state == 0)
	{
		game_state = 2;
		initTime = curtime;
		cam.update(vec3(0, 0, 300.0f));
	}*/

	if (game_state == 2){
		//printf("mouse clicked\n");
		tb.button = button;
		tb.mods = mods;
		if (action == GLFW_PRESS)			tb.begin(cam.view_matrix, npos);
		else if (action == GLFW_RELEASE)	tb.end();
	}
}

void motion(GLFWwindow* window, double x, double y)
{
	if (!tb.is_tracking()) return;

	if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update_track(npos);
	}

	/*else if (tb.button == GLFW_MOUSE_BUTTON_MIDDLE ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_CONTROL))) {
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update_pan(npos);

	}

	else if (tb.button == GLFW_MOUSE_BUTTON_RIGHT ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_SHIFT))) {
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update_zoom(npos);
	}*/

}


GLuint create_texture(const char* image_path, bool b_mipmap)
{
	// load the image with vertical flipping
	image* img = cg_load_image(image_path); if (!img) return -1;
	int w = img->width, h = img->height;

	// create a src texture (lena texture)
	GLuint texture; glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img->ptr);
	
	if (img) delete img; // release image

	// build mipmap
	if (b_mipmap && glGenerateMipmap){
			int mip_levels = 0; for (int k = max(w, h); k; k >>= 1) mip_levels++;
			for (int l = 1; l < mip_levels; l++)
				glTexImage2D(GL_TEXTURE_2D, l, GL_RGB8, max(1, w >> l), max(1, h >> l), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	// set up texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, b_mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

	return texture; 
}

bool user_init()
{
	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable(GL_BLEND);
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);			
	glActiveTexture(GL_TEXTURE0);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE1);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE2);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE3);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE4);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE5);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE6);		// notify GL the current texture slot is 0
	
	int temp;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &temp);
	printf("temp = %d\n", temp);

	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;

	//add sound source from the sound file
	btn_src = engine->addSoundSourceFromFile(sound_btn_path);
	shoot_src = engine->addSoundSourceFromFile(sound_shoot_path);
	hit_src = engine->addSoundSourceFromFile(sound_hit_path);
	success_src = engine->addSoundSourceFromFile(sound_success_path);
	fail_src = engine->addSoundSourceFromFile(sound_fail_path);
	bgm_src = engine->addSoundSourceFromFile(sound_bgm_path);
	miss_src = engine->addSoundSourceFromFile(sound_miss_path);

	//set default volume
	btn_src->setDefaultVolume(0.5f);
	shoot_src->setDefaultVolume(0.4f);
	hit_src->setDefaultVolume(0.35f);
	success_src->setDefaultVolume(0.5f);
	fail_src->setDefaultVolume(0.5f);
	bgm_src->setDefaultVolume(0.25f);
	miss_src->setDefaultVolume(0.5f);

	//play the sound file
	engine->play2D(bgm_src, true);
	
	// define the position of four corner vertices
	unit_vertices = std::move(create_vertices( ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_vertices);

	// load the image to a texture
	texture_Title = create_texture(texture_path_Title, true);		if (texture_Title == -1) return false;
	texture_Help = create_texture(texture_path_Help, true);			if (texture_Help == -1) return false;
	texture_Cleared = create_texture(texture_path_Cleared, true);	if (texture_Cleared == -1) return false;
	texture_Failed = create_texture(texture_path_Failed, true);		if (texture_Failed == -1) return false;
	texture_Virus = create_texture(texture_path_Virus, true);			if (texture_Virus == -1) return false;
	texture_Needle = create_texture(texture_path_Needle, true);			if (texture_Needle == -1) return false;
	texture_Prot = create_texture(texture_path_Prot, true);			if (texture_Prot == -1) return false;

	// setup freetype
	//if (!init_text()) return false;

	return true;
}

void user_finalize()
{
	// close the engine
	engine->drop();
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	spheres[0].center.x = 40.0f;
	spheres[1].center.x = 60.0f;
	spheres[2].center.x = 80.0f;

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
