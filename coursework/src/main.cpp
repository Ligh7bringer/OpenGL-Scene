#include <glm/glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;
using namespace std::chrono;

const unsigned int MAX_PARTICLES = 1000;

bool rain = false;
bool greyscale = false;
bool motion_blur = false;

// particles
struct particle {
	vec3 position = vec3(0, 0, 0);
	vec3 velocity = vec3(0, 0, 0);
};

// create particles
particle particles[MAX_PARTICLES];

//buffers we will need
// transform feedback buffers
GLuint transform_feedbacks[2];
// particle buffers
GLuint particle_buffers_vbo[2];

// VAO
GLuint vao;

// current buffer
unsigned int front_buf = 0;
// render buffer
unsigned int back_buf = 1;

//effects
effect eff, sky_eff, normal_map_eff, particle_eff, colour_eff, greyscale_eff, tarnish_eff, motion_blur_eff;

//camera
free_camera cam;

//cursor position for camera
double cursor_x = 0.0;
double cursor_y = 0.0;

//textures
array<texture, 20> texs;
texture bark, house, plane, bark_normal_map, wood_normal_map, grass_normal_map, tarnish;

//map for meshes
map<string, mesh> meshes;
map<string, mesh> nmapobj;

//light
spot_light spot;
vector<point_light> points(2);

//cubemap for the skybox
cubemap cube_map;
mesh skybox;
mesh moon;

//frame buffer used for post processing
frame_buffer frame;
geometry screen_quad;

//fram buffers needed for motion blur
frame_buffer frames[2];
frame_buffer temp_frame;
unsigned int current_frame = 0;

bool initialiase() {
	//set point size
	glPointSize(3.0f);
	// hide the pointer
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// get initial mouse x and y
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	renderer::setClearColour(0, 0, 0);

	return true;
}

bool load_content() {
	// initialise frame buffer
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// more frame buffers
	frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// temp framebuffer
	temp_frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	// Create screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

	//skybox
	auto boxgeom = geometry_builder::create_box();
	skybox = mesh(boxgeom);
	skybox.get_transform().scale = vec3(100, 100, 100);

	//moon
	auto spheregeom = geometry_builder::create_sphere();
	moon = mesh(spheregeom);

	//plane
	auto planegeom = geometry_builder::create_plane();
	nmapobj["plane"] = mesh(planegeom);

	//meshes for the scene
	meshes["axe"].set_geometry(geometry("models/axe.obj"));
	auto pathgeom = geometry_builder::create_box(vec3(60, 1, 15));
	meshes["path"] = mesh();
	auto barrelgeom = geometry_builder::create_cylinder(20, 20);
	meshes["barrel"] = mesh(barrelgeom);
	meshes["chair"].set_geometry(geometry("models/chair4.obj"));
	meshes["mill"].set_geometry(geometry("models/windmill.obj"));
	auto lampgeom = geometry("models/lamp.obj");
	meshes["lamp"] = mesh(lampgeom);
	auto lamp2geom = geometry("models/lamp.obj");
	meshes["lamp2"] = mesh(lamp2geom);
	nmapobj["tree1"].set_geometry(geometry("models/DeadTree.obj"));
	nmapobj["tree2"].set_geometry(geometry("models/DeadTree.obj"));
	nmapobj["house"].set_geometry(geometry("models/house.obj"));

	//set materials
	nmapobj["plane"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	nmapobj["plane"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	nmapobj["plane"].get_material().set_specular(vec4(0.5f, 0.35f, 0.05f, 1.0f));
	nmapobj["plane"].get_material().set_shininess(5.0f);
	//house
	nmapobj["house"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	nmapobj["house"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	nmapobj["house"].get_material().set_specular(vec4(0.5f, 0.35f, 0.05f, 1.0f));
	nmapobj["house"].get_material().set_shininess(5.0f);
	//axe
	meshes["axe"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["axe"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["axe"].get_material().set_specular(vec4(0.5f, 0.35f, 0.05f, 1.0f));
	meshes["axe"].get_material().set_shininess(10.0f);
	//path
	meshes["path"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["path"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["path"].get_material().set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["path"].get_material().set_shininess(10.0f);
	//barrel
	meshes["barrel"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["barrel"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["barrel"].get_material().set_specular(vec4(0.545f, 0.271f, 0.075f, 1.0f));
	meshes["barrel"].get_material().set_shininess(10.0f);
	//chair
	meshes["chair"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["chair"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["chair"].get_material().set_specular(vec4(0.545f, 0.271f, 0.075f, 1.0f));
	meshes["chair"].get_material().set_shininess(10.0f);
	//mill
	meshes["mill"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["mill"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["mill"].get_material().set_specular(vec4(0.871f, 0.722f, 0.529f, 1.0f));
	meshes["mill"].get_material().set_shininess(10.0f);
	//lamp
	meshes["lamp"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["lamp"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["lamp"].get_material().set_specular(vec4(0.827f, 0.827f, 0.827f, 1.0f));
	meshes["lamp"].get_material().set_shininess(20.0f);
	//lamp2
	meshes["lamp2"].set_material(meshes["lamp"].get_material());
	//tree
	nmapobj["tree1"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	nmapobj["tree1"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	nmapobj["tree1"].get_material().set_specular(vec4(0.827f, 0.827f, 0.827f, 1.0f));
	nmapobj["tree1"].get_material().set_shininess(10.0f);
	//tree2
	nmapobj["tree2"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	nmapobj["tree2"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	nmapobj["tree2"].get_material().set_specular(vec4(0.663f, 0.663f, 0.663f, 1.0f));
	nmapobj["tree2"].get_material().set_shininess(10.0f);
	//moon
	moon.get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	moon.get_material().set_emissive(vec4(0.753f, 0.753f, 0.753f, 1.0f));
	moon.get_material().set_specular(vec4(0.663f, 0.663f, 0.663f, 1.0f));
	moon.get_material().set_shininess(15.0f);

	//mesh transformations
	//house
	nmapobj["house"].get_transform().position = vec3(20.0f, 0.0f, 10.0f);
	nmapobj["house"].get_transform().orientation = vec3(0.0f, half_pi<float>(), 0.0f);
	//tree
	nmapobj["tree1"].get_transform().position = vec3(10, 0, 40);
	nmapobj["tree1"].get_transform().scale = vec3(1.5f, 1.5f, 1.5f);
	//axe
	meshes["axe"].get_transform().position = vec3(11.5f, 4.0f, 37);
	meshes["axe"].get_transform().scale = vec3(8.0f, 8.0f, 8.0f);
	meshes["axe"].get_transform().orientation = vec3(half_pi<float>(), 2 * pi<float>() / 45.0f, half_pi<float>());
	//path
	meshes["path"].get_transform().position.x -= 20;
	meshes["path"].get_transform().position.z += 10;
	//barrel
	meshes["barrel"].get_transform().scale = vec3(3.0f, 4.0f, 3.0f);
	meshes["barrel"].get_transform().position = vec3(15, 2, -2);
	//another tree
	nmapobj["tree2"].get_transform().position = vec3(29, 0, -25);
	nmapobj["tree2"].get_transform().scale = vec3(2.0f, 2.0f, 2.0f);
	nmapobj["tree2"].get_transform().rotate(vec3(0, pi<float>(), 0));
	//chair
	meshes["chair"].get_transform().position = vec3(9.0f, 1.0f, 7.0f);
	meshes["chair"].get_transform().orientation = vec3(0.0f, pi<float>(), 0.0f);
	//mill
	meshes["mill"].get_transform().position = vec3(-20, 0, -20);
	meshes["mill"].get_transform().scale = vec3(3.0f, 4.0f, 3.0f);
	//lamp2
	meshes["lamp2"].get_transform().position += vec3(0.0f, 0.0f, 20.0f);
	//moon
	moon.get_transform().scale = vec3(20.0f, 20.0f, 20.0f);
	moon.get_transform().position = vec3(80, 80, 80);

	//textures
	bark = texture("textures/bark.jpg");
	house = texture("textures/house.jpg");
	plane = texture("textures/grass01.jpg");
	tarnish = texture("textures/tarnish.jpg");
	bark_normal_map = texture("textures/treenormalmap.jpg");
	wood_normal_map = texture("textures/woodnormalmap.jpg");
	grass_normal_map = texture("textures/grass01_n.jpg");

	texs[0] = texture("textures/axe.png");
	texs[1] = texture("textures/barrel2.jpg");
	texs[2] = texture("textures/wood3.JPG");
	texs[3] = texture("textures/metal.jpg");
	texs[4] = texture("textures/metal.jpg");
	texs[5] = texture("textures/rust.jpg");
	texs[6] = texture("textures/pavement.jpg");

	//load skybox textures
	array<string, 6> filenames
	{
		"textures/cubemap/rt.tga", "textures/cubemap/lf.tga", "textures/cubemap/up2.jpg",
		"textures/cubemap/dn.tga", "textures/cubemap/bk.tga", "textures/cubemap/ft.tga"
	};

	//create cubemap
	cube_map = cubemap(filenames);

	//set light properties
	spot.set_position(moon.get_transform().position);
	spot.set_light_colour(vec4(0.416f, 0.353f, 0.804f, 1.0f));
	spot.set_direction(normalize(-spot.get_position()));
	spot.set_range(500.0f);
	spot.set_power(5.0f);

	points[0].set_position(meshes["lamp"].get_transform().position + vec3(0, 10.0f, 0));
	points[0].set_light_colour(vec4(1.0f, 0.843f, 0.0f, 1.0f));
	points[0].set_range(30.0f);

	points[1].set_position(meshes["lamp2"].get_transform().position + vec3(0, 10.0f, 0));
	points[1].set_light_colour(points[0].get_light_colour());
	points[1].set_range(30.0f);

	// Load shaders
	//main effect
	eff.add_shader("shaders/main.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/main.frag", GL_FRAGMENT_SHADER);
	//eff.add_shader("shaders/greyscale.frag", GL_FRAGMENT_SHADER);
	//normal map effect
	normal_map_eff.add_shader("shaders/lights.frag", GL_FRAGMENT_SHADER);
	normal_map_eff.add_shader("shaders/part_normal_map.frag", GL_FRAGMENT_SHADER);
	normal_map_eff.add_shader("shaders/shader.frag", GL_FRAGMENT_SHADER);
	normal_map_eff.add_shader("shaders/shader.vert", GL_VERTEX_SHADER);
	//skybox effect
	sky_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	//shaders used to colour the particles
	colour_eff.add_shader("shaders/basic_colour.frag", GL_FRAGMENT_SHADER);
	colour_eff.add_shader("shaders/basic_colour.vert", GL_VERTEX_SHADER);
	//particle eff
	particle_eff.add_shader("shaders/particle.frag", GL_FRAGMENT_SHADER);
	particle_eff.add_shader("shaders/particle.vert", GL_VERTEX_SHADER);
	particle_eff.add_shader("shaders/particle.geom", GL_GEOMETRY_SHADER);
	//greyscale eff
	greyscale_eff.add_shader("shaders/greyscale.frag", GL_FRAGMENT_SHADER);
	greyscale_eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	//tarnish effect
	tarnish_eff.add_shader("shaders/tarnish.frag", GL_FRAGMENT_SHADER);
	tarnish_eff.add_shader("shaders/tarnish.vert", GL_VERTEX_SHADER);
	//motion blur effect
	motion_blur_eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	motion_blur_eff.add_shader("shaders/motion_blur.frag", GL_FRAGMENT_SHADER);

	// Build effects
	eff.build();
	normal_map_eff.build();
	sky_eff.build();
	colour_eff.build();
	particle_eff.build();
	greyscale_eff.build();
	tarnish_eff.build();
	motion_blur_eff.build();

	//random generator and distribution for the particles
	default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	uniform_real_distribution<float> dist;

	// create particles
	for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
		particles[i].position = vec3(((150.0f * dist(rand)) - 50.0f), ((100.0f * dist(rand)) - 5.0f), ((150 * dist(rand)) - 50.0f));
		particles[i].velocity = vec3(0.0f, 50.0f + dist(rand), 0.0f);
	}

	// Use the particle effect
	renderer::bind(particle_eff);
	// describe output 
	const GLchar *attrib_names[2] = { "position_out", "velocity_out" };
	glTransformFeedbackVaryings(particle_eff.get_program(), 2, attrib_names, GL_INTERLEAVED_ATTRIBS);

	// Relink 
	glLinkProgram(particle_eff.get_program());

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Gen our VBO data buffers
	glGenBuffers(2, particle_buffers_vbo);
	// Place initial particle data in buffer 1
	glBindBuffer(GL_ARRAY_BUFFER, particle_buffers_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);

	// Fill space with blank data in buffer 2
	glBindBuffer(GL_ARRAY_BUFFER, particle_buffers_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particles), NULL, GL_DYNAMIC_DRAW);

	// generate our feedback objects
	glGenTransformFeedbacks(2, transform_feedbacks);
	// link fb[0] to vbo[1]
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedbacks[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_buffers_vbo[1]);
	// link fb[1] to vbo[0]
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedbacks[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_buffers_vbo[0]);

	// Set camera properties
	cam.set_position(vec3(-50.0f, 10.0f, 0.0f));
	cam.set_target(vec3(20.0f, 0.0f, 10.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	cout << "Press R to toggle rain and night on/off" << endl;
	cout << "Press G to toggle sepia on/off" << endl;
	cout << "Press B to toggle motion blur on/off" << endl;
	return true;
}

bool update(float delta_time) {
	// Flip frame
	current_frame = (current_frame + 1) % 2;

	if (rain) {
		//check if first frame
		static bool first_frame = true;
		//bind effect
		renderer::bind(particle_eff);
		//set delta_time uniform
		glUniform1f(particle_eff.get_uniform_location("delta_time"), delta_time);
	}

	moon.get_transform().rotate(vec3(0, quarter_pi<float>() / 2 * delta_time, 0));

	// The ratio of pixels to rotation
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	//variable where mouse position will be stored
	double current_x;
	double current_y;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios
	delta_x = delta_x*ratio_width;
	delta_y = delta_y*ratio_height;

	// Rotate camera
	cam.rotate(delta_x, -delta_y);

	// keyboard controls for camera
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W))
	{
		cam.move(vec3(0.0f, 0.0f, 1.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S))
	{
		cam.move(vec3(0.0f, 0.0f, -1.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A))
	{
		cam.move(vec3(-1.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D))
	{
		cam.move(vec3(1.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_SPACE))
	{
		cam.move(vec3(0.0f, 1.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT) )
	{
		cam.move(vec3(0.0f, -1.0f, 0.0f));
	}

	//go to the 4 corners of the plane
	if (glfwGetKey(renderer::get_window(), '1')) {
		cam.set_position(vec3(50, 10, 50));
	}
	if (glfwGetKey(renderer::get_window(), '2')) {
		cam.set_position(vec3(-50, 10, 50));
	}
	if (glfwGetKey(renderer::get_window(), '3')) {
		cam.set_position(vec3(-50, 10, -50));
	}
	if (glfwGetKey(renderer::get_window(), '4')) {
		cam.set_position(vec3(50, 10, -50));
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_R)) {
		rain = !rain;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_G)) {
		greyscale = !greyscale;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_B)) {
		motion_blur = !motion_blur;
	}
	// old cursor position becomes current
	cursor_x = current_x;
	cursor_y = current_y;

	// Set skybox position to camera position
	skybox.get_transform().position = cam.get_position();

	// Update the camera
	cam.update(delta_time);

	return true;
}

void render_feeback() {
	static bool first_frame = true;
	renderer::bind(particle_eff);

	// Close down the output
	glEnable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, particle_buffers_vbo[front_buf]);
	{
		// unfortunatly we can't save this stuff in the VAO. Because OGL.
		GLint position_in_loc = glGetAttribLocation(particle_eff.get_program(), "position_in");
		GLint velocity_in_loc = glGetAttribLocation(particle_eff.get_program(), "velocity_in");
		glEnableVertexAttribArray(position_in_loc);
		glEnableVertexAttribArray(velocity_in_loc);
		glVertexAttribPointer(position_in_loc, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (const GLvoid *)0);
		glVertexAttribPointer(velocity_in_loc, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (const GLvoid *)12);
	}

	// Tell openGL that we are rendering to feedback
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedbacks[front_buf]);
	glBeginTransformFeedback(GL_POINTS);

	if (first_frame) {
		// compute our points from the VBO like normal
		glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
		first_frame = false;
	}
	else {
		// compute from feedback data
		glDrawTransformFeedback(GL_POINTS, transform_feedbacks[back_buf]);
	}

	// stop feedback recording.
	glEndTransformFeedback();

	// reenable output
	glDisable(GL_RASTERIZER_DISCARD);
}

bool render() {

	if (motion_blur) {
		// Set render target to temp frame
		renderer::set_render_target(temp_frame);
		// Clear frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	if (greyscale) {
		// Set render target to frame buffer
		renderer::set_render_target(frame);
		// Clear frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// precalculate some of the matrices we will need
	auto P = cam.get_projection();
	auto V = cam.get_view();
	auto PV = P * V;

	// Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	// Bind skybox effect
	renderer::bind(sky_eff);

	// Calculate MVP for the skybox
	auto M = skybox.get_transform().get_transform_matrix();
	auto MVP = PV * M;

	// Set MVP matrix uniform
	glUniformMatrix4fv(sky_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(sky_eff.get_uniform_location("cubemap"), 0);

	// Render skybox
	renderer::render(skybox);

	// Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	if (rain) {
		render_feeback();
		renderer::bind(colour_eff);

		// Set the MVP matrix
		auto M = mat4(1.0f);
		auto MVP = PV * M;
		glUniformMatrix4fv(colour_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Set the colour uniform
		glUniform4fv(colour_eff.get_uniform_location("colour"), 1, value_ptr(vec4(0.0f, 0.0f, 1.0f, 0.5f)));

		// Bind the back particle buffer for rendering
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffers_vbo[front_buf]);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle), 0);

		// Perform the render by drawing the transform feedback
		glDrawTransformFeedback(GL_POINTS, transform_feedbacks[front_buf]);
		// Disable vertex attribute array
		glDisableVertexAttribArray(0);

		// Swap front and back buffers
		front_buf = back_buf;
		back_buf = (back_buf + 1) % 2;

	}

	// Bind effect
	renderer::bind(normal_map_eff);
	auto MVPloc = normal_map_eff.get_uniform_location("MVP");
	auto Mloc = normal_map_eff.get_uniform_location("M");
	auto Nloc = normal_map_eff.get_uniform_location("N");
	auto eyeloc = normal_map_eff.get_uniform_location("eye_pos");
	auto texloc = normal_map_eff.get_uniform_location("tex");
	auto nmloc = normal_map_eff.get_uniform_location("normal_map");
	// Bind light
	renderer::bind(spot, "spot");
	renderer::bind(points, "points");

	int j = 0;
	for (auto &e : nmapobj) {
		auto m = e.second;

		mat4 M = m.get_transform().get_transform_matrix();
		auto MVP = PV * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(MVPloc, 1, GL_FALSE, value_ptr(MVP));
		// Set M matrix uniform
		glUniformMatrix4fv(Mloc, 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(Nloc, 1, GL_FALSE,
			value_ptr(m.get_transform().get_normal_matrix()));

		// Bind material
		renderer::bind(m.get_material(), "mat");

		if (j == 0) {
			// Bind texture
			renderer::bind(house, 0);
			// Set tex uniform
			glUniform1i(texloc, 0);
			// Bind normal_map
			renderer::bind(wood_normal_map, 1);
			// Set normal_map uniform
			glUniform1i(nmloc, 1);
		}
		else if (j == 1) {
			// Bind texture
			renderer::bind(plane, 0);
			// Set tex uniform
			glUniform1i(texloc, 0);
			// Bind normal_map
			renderer::bind(grass_normal_map, 1);
			// Set normal_map uniform
			glUniform1i(nmloc, 1);
		}
		else {
			// Bind texture
			renderer::bind(bark, 0);
			// Set tex uniform
			glUniform1i(texloc, 0);
			// Bind normal_map
			renderer::bind(bark_normal_map, 1);
			// Set normal_map uniform
			glUniform1i(nmloc, 1);
		}
		// Set eye position
		glUniform3fv(eyeloc, 1, value_ptr(cam.get_position()));
		// Render mesh
		renderer::render(m);
		j++;
	}

	renderer::bind(eff);
	MVPloc = eff.get_uniform_location("MVP");
	Mloc = eff.get_uniform_location("M");
	Nloc = eff.get_uniform_location("N");
	eyeloc = eff.get_uniform_location("eye_pos");
	texloc = eff.get_uniform_location("tex");
	// bind light
	renderer::bind(spot, "spot");
	renderer::bind(points, "points");

	// accumulator to take care of textures and their ids when passing to the shader
	int i = 0;
	//for-each loop to take care of all meshes
	for (auto &e : meshes) {
		//get mesh
		auto m = e.second;

		// Create MVP matrix
		mat4 M = m.get_transform().get_transform_matrix();
		auto MVP = PV * M;

		// Set MVP uniform
		glUniformMatrix4fv(MVPloc, 1, GL_FALSE, value_ptr(MVP));
		// set M uniform
		glUniformMatrix4fv(Mloc, 1, GL_FALSE, value_ptr(M));
		// Set N uniform
		glUniformMatrix3fv(Nloc, 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

		// bind material
		renderer::bind(m.get_material(), "mat");
		//bind texture
		renderer::bind(texs[i], i);
		//set tex uniform
		glUniform1i(texloc, i);
		// set eye position
		glUniform3fv(eyeloc, 1, value_ptr(cam.get_position()));

		// render mesh
		renderer::render(m);

		//dont forget to increment i
		i++;
	}

	// Bind effect
	renderer::bind(tarnish_eff);
	// Create MVP matrix
	M = moon.get_transform().get_transform_matrix();
	MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(tarnish_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(tarnish_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(tarnish_eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(moon.get_transform().get_normal_matrix()));
	// Set eye_pos value
	glUniform3fv(tarnish_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// Bind cubemap to TU 0
	renderer::bind(cube_map, 0);
	// Set cubemap uniform
	glUniform1i(tarnish_eff.get_uniform_location("cubemap"), 0);
	// Bind tarnish to TU 1
	renderer::bind(tarnish, 1);
	// Set tarnish uniform to 1
	glUniform1i(tarnish_eff.get_uniform_location("tex"), 1);
	// Render mesh
	renderer::render(moon);

	if (greyscale) {
		renderer::set_render_target();
		// Bind Tex effect
		renderer::bind(greyscale_eff);
		// MVP is now the identity matrix
		MVP = mat4(1.0f);
		// Set MVP matrix uniform
		glUniformMatrix4fv(greyscale_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from frame buffer
		renderer::bind(frame.get_frame(), 0);
		// Set the tex uniform
		glUniform1i(greyscale_eff.get_uniform_location("tex"), 0);
		// Render the screen quad
		renderer::render(screen_quad);
	}

	if (motion_blur) {
		// Set render target to current frame
		renderer::set_render_target(frames[current_frame]);
		// Clear frame
		glClear(GL_DEPTH_BUFFER_BIT);
		// Bind motion blur effect
		renderer::bind(motion_blur_eff);
		// MVP is now the identity matrix
		mat4 MVP = mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		// Set MVP matrix uniform
		glUniformMatrix4fv(motion_blur_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind tempframe to TU 0.
		renderer::bind(temp_frame.get_frame(), 0);
		// Bind frames[(current_frame + 1) % 2] to TU 1.
		renderer::bind(frames[(current_frame + 1) % 2].get_frame(), 1);
		// Set tex uniforms
		glUniform1i(motion_blur_eff.get_uniform_location("previous_frame"), 1);
		glUniform1i(motion_blur_eff.get_uniform_location("tex"), 0);
		// Set blend factor (0.9f)
		glUniform1f(motion_blur_eff.get_uniform_location("blend_factor"), 0.9f);
		// Render screen quad
		renderer::render(screen_quad);

		// Set render target back to the screen
		renderer::set_render_target();
		// Set MVP matrix uniform
		glUniformMatrix4fv(motion_blur_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from frame buffer
		renderer::bind(frames[current_frame].get_frame(), 3);
		// Set the uniform
		glUniform1i(motion_blur_eff.get_uniform_location("tex"), 3);
		// Render the screen quad
		renderer::render(screen_quad);
	}

	return true;
}



int main() {
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_initialise(initialiase);
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();

	return EXIT_SUCCESS;
}