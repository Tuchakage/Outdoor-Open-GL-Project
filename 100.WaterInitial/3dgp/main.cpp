#define _USE_MATH_DEFINES
#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"
#include <cmath>


// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglTerrain terrain, water;

//Programs
C3dglProgram ProgramBasic, ProgramWater, ProgramTerrain, ProgramParticle;

//Textures
GLuint idTexGrass, idTexSand, idTexNone, idTexParticle, idTexWater, idTexCube;
GLuint idBufferVelocity, idBufferStartTime, idBufferPosition;
// Water specific variables
float waterLevel = 4.6f;

// Particle System Params
const float PERIOD = 0.00075f;
const float LIFETIME = 6;
const int NPARTICLES = (int)(LIFETIME / PERIOD);


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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// setup the point size
	glEnable(0x8642);
	glEnable(GL_POINT_SPRITE);




	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;
	C3dglBitmap grasstex, sandtex,particletex, watertex;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramBasic.Create()) return false;
	if (!ProgramBasic.Attach(VertexShader)) return false;
	if (!ProgramBasic.Attach(FragmentShader)) return false;
	if (!ProgramBasic.Link()) return false;
	if (!ProgramBasic.Use(true)) return false;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(VertexShader)) return false;
	if (!ProgramTerrain.Attach(FragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;


	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/particle.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/particle.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramParticle.Create()) return false;
	if (!ProgramParticle.Attach(VertexShader)) return false;
	if (!ProgramParticle.Attach(FragmentShader)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(ProgramBasic.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(ProgramBasic.GetAttribLocation("aNormal"));


	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap.png", 10)) return false;
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;


	//Load Textures here!
	grasstex.Load("models/grass.png", GL_RGBA);
	if (!grasstex.GetBits()) return false;

	sandtex.Load("models/sand.png", GL_RGBA);
	if (!sandtex.GetBits()) return false;

	particletex.Load("models/particle1.bmp", GL_RGBA);
	if (!particletex.GetBits()) return false;

	watertex.Load("models/water.png", GL_RGBA);
	if (!watertex.GetBits()) return false;

	//Preparing Texture Buffer For Grass
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grasstex.GetWidth(), grasstex.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, grasstex.GetBits());

	//Preparing Texture Buffer For Sand
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sandtex.GetWidth(), sandtex.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, sandtex.GetBits());

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexParticle);
	glBindTexture(GL_TEXTURE_2D, idTexParticle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, particletex.GetWidth(), particletex.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, particletex.GetBits());

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, watertex.GetWidth(), watertex.GetHeight(), 0, GL_RGBA,

		GL_UNSIGNED_BYTE, watertex.GetBits());

	// load Cube Map
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);


	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramBasic.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	ProgramBasic.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	ProgramTerrain.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	ProgramTerrain.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	// setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	ProgramTerrain.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramTerrain.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramWater.SendUniform("skyColor", 0.47f, 0.4324f, 0.4324f);
	ProgramTerrain.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramTerrain.SendUniform("waterLevel", waterLevel);

	ProgramTerrain.SendUniform("fogDensity", 0.3f);
	ProgramTerrain.SendUniform("fogDensity2", 0.1f);
	ProgramTerrain.SendUniform("fogColour", 0.47f, 0.4324f, 0.4324f);

	// Setup the particle system
	//Initial Position
	//ProgramParticle.SendUniform("initialPos", 0.0, 10.0, 0.0); 
	//Gravity
	//ProgramParticle.SendUniform("gravity", 0.0, -0.2, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);

	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	std::vector<float> bufferPosition;

	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = (float)M_PI / 6.f * (float)rand() / (float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 2 + 0.5f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x);
		bufferVelocity.push_back(-y);
		bufferVelocity.push_back(z);

		bufferPosition.push_back(x * v);
		bufferPosition.push_back(y);
		bufferPosition.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],
		GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],
		GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferPosition);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferPosition.size(), &bufferPosition[0],
	GL_STATIC_DRAW);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 0.4, 30.0),
		vec3(4.0, 0.4, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	//glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour
	glClearColor(0.47f, 0.4324f, 0.4324f, 1.0f); // Foggy Sky
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Sky
	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}



void renderWater(mat4 matrixView, float theta)
{
	mat4 m;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	ProgramWater.SendUniform("texture0", 0);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	ProgramWater.SendUniform("textureCubeMap", 1);
	ProgramWater.SendUniform("reflectionPower", 1.0);

	// render the water
	ProgramWater.Use();
	m = matrixView;
	m = translate(m, vec3(0, waterLevel, 0));
	m = scale(m, vec3(0.5f, 1.0f, 0.5f));
	ProgramWater.SendUniform("matrixModelView", m);
	water.render(m);
}

void renderObjects(mat4 matrixView, float theta)
{

	mat4 m;

	// render the terrain
	ProgramTerrain.Use();

	// setup the textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	ProgramTerrain.SendUniform("textureBed", 1);
	ProgramWater.SendUniform("reflectionPower", 0.0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	ProgramTerrain.SendUniform("textureShore", 2);


	m = matrixView;
	terrain.render(m);

	//No Texture
	//glBindTexture(GL_TEXTURE_2D, idTexNone);


	glDepthMask(GL_FALSE);				// disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);			// choose the active texture
	glBindTexture(GL_TEXTURE_2D, idTexParticle);	// bind the texture

	ProgramParticle.SendUniform("texture0", 0);
	// RENDER THE PARTICLE SYSTEM
	ProgramParticle.Use();

	m = matrixView;
	m = scale(m, vec3(50.5f, 50.5f, 50.5f));
	//m = translate(m, vec3(0.1, 0.0, 0.5));
	//m = translate(m, cam);
	ProgramParticle.SendUniform("matrixModelView", m);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
	glEnableVertexAttribArray(2);	// position
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferPosition);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glDepthMask(GL_TRUE);		// don't forget to switch the depth test updates back on
}

void prepareCubeMap(float x, float y, float z, float theta)
{
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	// setup the viewport to 256x256, 90 degrees FoV (Field of View)
	glViewport(0, 0, 256, 256);
	ProgramWater.SendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));

	// render environment 6 times
	ProgramWater.SendUniform("reflectionPower", 0.0);
	for (int i = 0; i < 6; ++i)
	{
		// clear background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// setup the camera
		const GLfloat ROTATION[6][6] =
		{	// at              up
			{ 1.0, 0.0, 0.0,   0.0, -1.0, 0.0 },  // pos x
			{ -1.0, 0.0, 0.0,  0.0, -1.0, 0.0 },  // neg x
			{ 0.0, 1.0, 0.0,   0.0, 0.0, 1.0 },   // pos y
			{ 0.0, -1.0, 0.0,  0.0, 0.0, -1.0 },  // neg y
			{ 0.0, 0.0, 1.0,   0.0, -1.0, 0.0 },  // poz z
			{ 0.0, 0.0, -1.0,  0.0, -1.0, 0.0 }   // neg z
		};
		mat4 matrixView2 = lookAt(
			vec3(x, y, z),
			vec3(x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2]),
			vec3(ROTATION[i][3], ROTATION[i][4], ROTATION[i][5]));

		// send the View Matrix
		ProgramWater.SendUniform("matrixView", matrixView);

		// render scene objects - all but the reflective one
		glActiveTexture(GL_TEXTURE0);
		renderObjects(matrixView2, theta);
		renderWater(matrixView2, theta);

		// send the image to the cube texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
	}

	// restore the matrixView, viewport and projection
	void reshape(int w, int h);
	reshape(w, h);
}

void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	//prepareCubeMap(0.0f, 0.2f, 0.0f, theta);

	// send the animation time to shaders
	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);
	ProgramParticle.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);
	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on

	matrixView = m * matrixView;

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -std::max(terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]), waterLevel);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	// setup View Matrix
	ProgramBasic.SendUniform("matrixView", matrixView);
	ProgramWater.SendUniform("matrixView", matrixView);
	ProgramTerrain.SendUniform("matrixView", matrixView);
	ProgramParticle.SendUniform("matrixView", matrixView);

	vec3 n(-6.4f, 3.86f, -2.5f);
	vec3 p(-6.4f, 3.86f, -2.5f);

	//reflection matrix
	float a = n.x, b = n.y, c = n.z, d = -dot(p, n); //parameters of the reflection plane: Ax + By + Cz + d = 0
	mat4 matrixReflection = mat4(
		1 - 2*a*a, -2*a*b, -2*a*c,0,
		-2 * a*b, 1 - 2*b*b, 02 * b *c,0,
		-2*a*c, -2*b*c,1-2*c*c,0,
		-2*a*d, -2*b*d, -2 * c*d,1);

	//Prepare the Stencil Test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	//Just render the water, only with stencil output
	renderWater(matrixView, theta);

	//setup stencil to the render the reflection
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	//setup the view matrix for the reflected image
	matrixView *= matrixReflection;
	ProgramWater.SendUniform("matrixView", matrixView);

	ProgramWater.SendUniform("planeClip", a, b, c, d);
	glEnable(GL_CLIP_PLANE0);

	//render the reflection
	renderObjects(matrixView, theta);

	//switch off the stencil and the clip plane
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_PLANE0);

	//revert to normal view matrix
	matrixView *= matrixReflection;
	ProgramWater.SendUniform("matrixView", matrixView);

	//render the image
	//first render the mirror to satisfy the depth test
	//but block the colour output so that the reflection remains on screen
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	renderWater(matrixView, theta);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//render everything else as normally
	renderObjects(matrixView, theta);

	//Trying to get Rain to be around Camera
	//std::vector<float> bufferPosition;

	//for (int i = 0; i < NPARTICLES; i++)
	//{

	//	float theta = (float)M_PI / 6.f * (float)rand() / (float)RAND_MAX;
	//	float phi = (float)M_PI * 2.f * (float)rand() / (float)RAND_MAX;
	//	float x = sin(theta) * cos(phi);
	//	float y = cos(theta);
	//	float z = sin(theta) * sin(phi);
	//	float v = 2 + 0.5f * (float)rand() / (float)RAND_MAX;
	//	bufferPosition.push_back(cam.x * v);
	//	bufferPosition.push_back(y);
	//	bufferPosition.push_back(cam.z * v);
	//}

	//glGenBuffers(1, &idBufferPosition);
	//glBindBuffer(GL_ARRAY_BUFFER, idBufferPosition);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferPosition.size(), &bufferPosition[0],
	//	GL_STATIC_DRAW);

	//renderWater(matrixView, theta);
	//renderObjects(matrixView, theta);
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
	mat4 m = perspective(radians(60.f), ratio, 0.02f, 1000.f);
	ProgramBasic.SendUniform("matrixProjection", m);
	ProgramWater.SendUniform("matrixProjection", m);
	ProgramTerrain.SendUniform("matrixProjection", m);
	ProgramParticle.SendUniform("matrixProjection", m);
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

