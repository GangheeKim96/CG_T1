#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "circle.h"		// circle class definition
#include "trackball.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//*************************************
// global constants
static const char*	window_name = "cgbase - circle";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";


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

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices

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
	vec4	position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
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

auto	circles = std::move(create_circles());
camera cam;
trackball tb;
light_t		light;
material_t	material;
float curtime = 0.0f;
float initTime = 0.0f;
int game_state = 0;

static const char* texture_path_Title = "../bin/textures/title.jpg";
static const char* texture_path_Help = "../bin/textures/help.jpg";
static const char* texture_path_Face = "../bin/textures/face.jpg";
GLuint texture_Title = 0;
GLuint texture_Help = 0;
GLuint texture_Face = 0;

void update()
{

	// update global simulation parameter
	curtime = float(glfwGetTime());


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
	glBindTexture(GL_TEXTURE_2D, texture_Face);
	glUniform1i(glGetUniformLocation(program, "TEX_FACE"), 2);

	// bind vertex array object
	glBindVertexArray( vertex_array );

	if (game_state == 2) {
		// render two circles: trigger shader program to process vertex data
		for (int i = 0; i < 12; i++)
		{
			circle_t* c = &(circles[i]);

			// per-circle update
			c->update(initTime, curtime, i);

			// update per-circle uniforms
			GLint uloc;
			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, c->model_matrix);
			glUniform1i(glGetUniformLocation(program, "planet_num"), i);

			// per-circle draw calls
			glDrawElements(GL_TRIANGLES, 72 * 36 * 6, GL_UNSIGNED_INT, nullptr);

		}
	}

	mat4 temp = { 1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1 };
	GLint uloc;
	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, temp);
	glUniform1i(glGetUniformLocation(program, "planet_num"), 100);
	glDrawArrays(GL_TRIANGLES, 73*36 + 72 + 1, 6); // (topology, start offset, no. vertices)

	glUniform1i(glGetUniformLocation(program, "planet_num"), 101);
	glDrawArrays(GL_TRIANGLES, 73 * 36 + 72 + 7, 6); // (topology, start offset, no. vertices)



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

std::vector<vertex> create_circle_vertices( void )
{
	std::vector<vertex> v;
	
	for( int i=0; i <= 36; i++ )
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

	vertex title[4];
	title[0].pos = vec3(-22.0f, -12.0f, 950.0f);	title[0].tex = vec2(0.0f, 0.0f);
	title[1].pos = vec3(+22.0f, -12.0f, 950.0f);	title[1].tex = vec2(1.0f, 0.0f);
	title[2].pos = vec3(+22.0f, +12.0f, 950.0f);	title[2].tex = vec2(1.0f, 1.0f);
	title[3].pos = vec3(-22.0f, +12.0f, 950.0f);	title[3].tex = vec2(0.0f, 1.0f);
	v.push_back(title[0]); v.push_back(title[1]); v.push_back(title[2]);
	v.push_back(title[0]); v.push_back(title[2]); v.push_back(title[3]);

	vertex help[4];
	help[0].pos = vec3(-22.0f, -12.0f, 850.0f);	help[0].tex = vec2(0.0f, 0.0f);
	help[1].pos = vec3(+22.0f, -12.0f, 850.0f);	help[1].tex = vec2(1.0f, 0.0f);
	help[2].pos = vec3(+22.0f, +12.0f, 850.0f);	help[2].tex = vec2(1.0f, 1.0f);
	help[3].pos = vec3(-22.0f, +12.0f, 850.0f);	help[3].tex = vec2(0.0f, 1.0f);
	v.push_back(help[0]); v.push_back(help[1]); v.push_back(help[2]);
	v.push_back(help[0]); v.push_back(help[2]); v.push_back(help[3]);

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
	
	for( int i=0; i<36; i++ )
	{
		for (int j = 0; j < 72; j++) {
			indices.push_back(73 * i + j);
			indices.push_back(73 * (i + 1) + j);
			indices.push_back(73 * i + j + 1);
			indices.push_back(73 * i + j + 1);
			indices.push_back(73 * (i + 1) + j);
			indices.push_back(73 * (i + 1) + j + 1);
		}
		
	}

	int cnt = 73 * 36 + 72 + 1;
	for (int i = 0; i < 12; i++) {
		indices.push_back(cnt + i);
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
			if (game_state == 0) {
				game_state = 1;
				cam.update(vec3(0.0f, 0.0f, 900.0f));
				//printf("cam.eye = (%f, %f, %f)\n", cam.eye[0], cam.eye[1], cam.eye[2]);
			}
			else if (game_state == 1) {
				game_state = 0;
				cam.update(vec3(0.0f, 0.0f, 1000.0f));
				//printf("cam.eye = (%f, %f, %f)\n", cam.eye[0], cam.eye[1], cam.eye[2]);
			}
		}
		
		else if (key == GLFW_KEY_R) {
			game_state = 0;
			for (int i = 0; i < 12; i++)
			{
				circle_t* c = &(circles[i]);

				// per-circle reset
				c->reset(i);

			}
			cam.update(vec3(0, 0, 1000.0f));
		}
	}
	
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && game_state == 0)
	{
		game_state = 2;
		initTime = curtime;
		cam.update(vec3(0, 0, 300.0f));
	}

	if (game_state == 2){
		printf("mouse clicked\n");
		tb.button = button;
		tb.mods = mods;
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = cursor_to_ndc(pos, window_size);
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

	else if (tb.button == GLFW_MOUSE_BUTTON_MIDDLE ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_CONTROL))) {
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update_pan(npos);

	}

	else if (tb.button == GLFW_MOUSE_BUTTON_RIGHT ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_SHIFT))) {
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update_zoom(npos);
	}

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
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable(GL_TEXTURE_2D);			// enable texturing
	glActiveTexture(GL_TEXTURE0);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE1);		// notify GL the current texture slot is 0
	glActiveTexture(GL_TEXTURE2);		// notify GL the current texture slot is 0

	
	int temp;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &temp);
	printf("temp = %d\n", temp);


	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices( ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_circle_vertices);

	// load the image to a texture
	texture_Title = create_texture(texture_path_Title, true);		if (texture_Title == -1) return false;
	texture_Help = create_texture(texture_path_Help, true);		if (texture_Help == -1) return false;
	texture_Face = create_texture(texture_path_Face, true);		if (texture_Face == -1) return false;

	return true;
}

void user_finalize()
{
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
