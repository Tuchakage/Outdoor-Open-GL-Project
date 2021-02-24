#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglTerrain terrain, road;
C3dglModel lamp;
//Skybox
C3dglSkyBox skybox, nightbox;
// GLSL Program
C3dglProgram Program;
//Textures
GLuint idTexGrass;
GLuint idTexNone;
GLuint idTexRoad;
//So we can switch between Day Light and Night time
int dayLight = 0;
//So we can turn on and off the attenuation
int atton = 1;
// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE! 

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;
	C3dglBitmap grassbmp,roadbmp;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;


	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap.bmp", 10)) return false;
	if (!road.loadHeightmap("models\\roadmap.bmp", 10)) return false;
	if (!lamp.load("models\\street lamp - fancy.obj")) return false;


	// load Sky Box     
	if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	if (!nightbox.load("models\\NightTime\\NightTimeFront1024.jpg",
		"models\\NightTime\\NightTimeLeft1024.jpg",
		"models\\NightTime\\NightTimeBack1024.jpg",
		"models\\NightTime\\NightTimeRight1024.jpg",
		"models\\NightTime\\NightTimeUp1024.jpg",
		"models\\NightTime\\NightTimeDown1024.jpg")) return false;

	//Load Textures here!
	grassbmp.Load("models/grass.png", GL_RGBA);
	if (!grassbmp.GetBits()) return false;

	roadbmp.Load("models/road.png", GL_RGBA);
	if (!roadbmp.GetBits()) return false;

	//Preparing Texture Buffer For Grass
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grassbmp.GetWidth(), grassbmp.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, grassbmp.GetBits());

	//Preparing Texture Buffer For Road
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexRoad);
	glBindTexture(GL_TEXTURE_2D, idTexRoad);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, roadbmp.GetWidth(), roadbmp.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, roadbmp.GetBits());



	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);


	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 1.5, 30.0),
		vec3(4.0, 1.5, 0.0),
		vec3(0.0, 1.0, 0.0));



	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << "  N to switch between day and night" << endl;
	cout << "  T to switch between attenuation on and off" << endl;
	cout << endl;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));



	return true;
}

void done()
{
}

void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;


	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	// setup View Matrix
	Program.SendUniform("matrixView", matrixView);



	if (dayLight == 1) 
	{
		glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // Light Blue Sky 
		// setup ambient light and material:
			//Turn on Both Ambient and Direction Light
		Program.SendUniform("lightAmbient.on", 1);
		Program.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);

		// setup directional light and the diffuse material:
		Program.SendUniform("lightDir1.on", 1);
		Program.SendUniform("lightDir1.direction", 0.75, 2.0, -1.0);
		Program.SendUniform("lightDir1.diffuse", 1.0, 1.0, 1.0);

		//Temporarily set the Ambient light and Ambient material to white and diffuse material to black (This gets rid of the box and makes the skybox bright)
		Program.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
		Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
		Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
		// render the skybox (Day Time)
		m = matrixView;
		skybox.render(m);
		Program.SendUniform("lightAmbient.color", 0.4, 0.4, 0.4);
		//When it is day time turn off the Lamp
		Program.SendUniform("lightPoint1.on", 0);
		Program.SendUniform("lightPoint2.on", 0);
		Program.SendUniform("lightPoint3.on", 0);
		Program.SendUniform("lightPoint4.on", 0);
		Program.SendUniform("lightPoint5.on", 0);
		Program.SendUniform("lightPoint6.on", 0);
		Program.SendUniform("lightAmbient2.on", 0);


	}
	else 
	{
		glClearColor(0.0329f, 0.01f, 0.0839f, 1.0f); // Dark Blue Sky 

		//Turn on Both Ambient and Direction Light
		Program.SendUniform("lightAmbient.on", 0);
		Program.SendUniform("lightDir1.on", 0);
		//Switch between Attenuation being on and off
		Program.SendUniform("atton", atton);

		//Point Light 1 (Diffuse)
		Program.SendUniform("lightPoint1.on", 1);
		Program.SendUniform("lightPoint1.position", 6.2f, 5.47f, 15.0f);
		Program.SendUniform("lightPoint1.diffuse", 0.4, 0.4, 0.4);
		//Point Light 1 (Specular Extension)
		Program.SendUniform("lightPoint1.specular", 1.0, 1.0, 1.0);

		//Point Light 2 (Diffuse)
		Program.SendUniform("lightPoint2.on", 1);
		Program.SendUniform("lightPoint2.position", 4.7f, 5.12f, 10.0f);
		Program.SendUniform("lightPoint2.diffuse", 0.4, 0.4, 0.4);
		//Point Light 2 (Specular Extension)
		Program.SendUniform("lightPoint2.specular", 1.0, 1.0, 1.0);

		//Point Light 3 (Diffuse)
		Program.SendUniform("lightPoint3.on", 1);
		Program.SendUniform("lightPoint3.position", 4.7f, 4.27f, 5.0f);
		Program.SendUniform("lightPoint3.diffuse", 0.4, 0.4, 0.4);
		//Point Light 3 (Specular Extension)
		Program.SendUniform("lightPoint3.specular", 1.0, 1.0, 1.0);

		//Point Light 4 (Diffuse)
		Program.SendUniform("lightPoint4.on", 1);
		Program.SendUniform("lightPoint4.position", 6.2f, 4.27f, 0.0f);
		Program.SendUniform("lightPoint4.diffuse", 0.4, 0.4, 0.4);
		//Point Light 4 (Specular Extension)
		Program.SendUniform("lightPoint4.specular", 1.0, 1.0, 1.0);

		//Point Light 5 (Diffuse)
		Program.SendUniform("lightPoint5.on", 1);
		Program.SendUniform("lightPoint5.position", 4.7f, 4.57f, -5.0f);
		Program.SendUniform("lightPoint5.diffuse", 0.4, 0.4, 0.4);
		//Point Light 5 (Specular Extension)
		Program.SendUniform("lightPoint5.specular", 1.0, 1.0, 1.0);

		//Point Light 6 (Diffuse)
		Program.SendUniform("lightPoint6.on", 1);
		Program.SendUniform("lightPoint6.position", 4.7f, 4.72f, 20.0f);
		Program.SendUniform("lightPoint6.diffuse", 0.4, 0.4, 0.4);
		//Point Light 6 (Specular Extension)
		Program.SendUniform("lightPoint6.specular", 1.0, 1.0, 1.0);

		Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
		Program.SendUniform("shininess", 3.0);

		//2nd Ambient Light To brighten up the bulb
		Program.SendUniform("lightAmbient2.on", 1);
		Program.SendUniform("lightAmbient2.color", 1.0, 1.0, 1.0);
		//Light Attentuation
		Program.SendUniform("attenuation", 0.2);

		//This line of code brightens the scene up a tiny bit, enough so that we can see the Night Sky
		Program.SendUniform("materialAmbient", 0.1, 0.1, 0.1);
		//This stops the shine that happens on the sky box when you get near the lamps
		Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
		// render the skybox (Night Time)
		m = matrixView;
		nightbox.render(m);
		//After the Skybox has finished rendering then set Material Diffuse to white so that point light will work on the scene properly
		Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	}




	//Bind Grass Texture To Terrain
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	// setup materials - green (grass)
	//Program.SendUniform("materialDiffuse", 0.2f, 0.8f, 0.2f);
	//Program.SendUniform("materialAmbient", 0.2f, 0.8f, 0.2f);
	// render the terrain
	m = translate(matrixView, vec3(0, 0, 0));
	terrain.render(m);


	glBindTexture(GL_TEXTURE_2D, idTexRoad);

	// setup materials - grey (road)
	Program.SendUniform("materialDiffuse", 0.3f, 0.3f, 0.16f);
	//Program.SendUniform("materialAmbient", 0.3f, 0.3f, 0.16f);
	// render the road
	m = translate(matrixView, vec3(0, 0, 0));
	m = translate(m, vec3(6.0f, 0.01f, 0.0f));
	road.render(m);
	//No Texture
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	
	//Black Material For Lamp
	Program.SendUniform("materialDiffuse", 0.0f, 0.0f, 0.0f);
	Program.SendUniform("materialAmbient", 0.0f, 0.0f, 0.0f);

	//Black Lamp 1
	m = translate(matrixView, vec3(6.2f, 4.25f, 15.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//Black Lamp 2
	m = translate(matrixView, vec3(4.7f, 3.9f, 10.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//Black Lamp 3
	m = translate(matrixView, vec3(4.7f, 3.05f, 5.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//Black Lamp 4
	m = translate(matrixView, vec3(6.2f, 3.05f, 0.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//Black Lamp 5
	m = translate(matrixView, vec3(4.7f, 3.35f, -5.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//Black Lamp 6
	m = translate(matrixView, vec3(4.7f, 3.5f, 20.0f));
	m = scale(m, vec3(0.0100f, 0.0100f, 0.0100f));
	lamp.render(m);

	//White Material For Light Bulbs
	Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("materialAmbient", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("materialSpecular", 1.0f, 1.0f, 1.0f);

	//Bulb 1
	m = matrixView;
	m = translate(matrixView, vec3(6.2f, 5.47f, 15.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb 2
	m = matrixView;
	m = translate(matrixView, vec3(4.7f, 5.12f, 10.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb 3
	m = matrixView;
	m = translate(matrixView, vec3(4.7f, 4.27f, 5.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb 4
	m = matrixView;
	m = translate(matrixView, vec3(6.2f, 4.27f, 0.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb 5
	m = matrixView;
	m = translate(matrixView, vec3(4.7f, 4.57f, -5.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb 6
	m = matrixView;
	m = translate(matrixView, vec3(4.7f, 4.72f, 20.0f));
	m = scale(m, vec3(0.125f, 0.125f, 0.125f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	//Change the colour to Black After the Bulbs have rendered
	Program.SendUniform("materialAmbient", 0.0f, 0.0f, 0.0f);





	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();

}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	Program.SendUniform("matrixProjection", matrixProjection);

}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case 'n': dayLight = 1 - dayLight; break;
	case 't': atton = 1 - atton; break;

	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

