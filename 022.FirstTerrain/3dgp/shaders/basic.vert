#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform float fogDensity;


in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float fogFactor;

// Light declarations
struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient, lightAmbient2;

//Calculates the Ambient Light
vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir1;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}


void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate light
	color = vec4(0, 0, 0, 1);
	normal = normalize(mat3(matrixModelView) * aNormal);


	// calculate texture coordinate
	texCoord0 = aTexCoord;

	//Calculating Fog Factor For Normal Fog
	fogFactor = exp2(-fogDensity * length(position)) ;

	//Contains The Colour Of The Light
	if (lightAmbient.on == 1)
	{
		color += AmbientLight(lightAmbient);
	}
	if (lightAmbient2.on == 1)
	{
		color += AmbientLight(lightAmbient2);
	}

	if (lightDir1.on == 1)
	{
		color += DirectionalLight(lightDir1);
	} 
}
