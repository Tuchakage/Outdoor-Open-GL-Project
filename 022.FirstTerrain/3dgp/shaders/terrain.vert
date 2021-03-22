#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
// Uniforms: Water Related
uniform float waterLevel;	// water level (in absolute units)
uniform float fogDensity, fogDensity2;


in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
// Output: Water Related
out float waterDepth;	// water depth (positive for underwater, negative for the shore)
out float fogFactor, fogFactor2;

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

	// calculate depth of water
	waterDepth = waterLevel - aVertex.y;

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	// calculate the observer's altitude above the observed vertex
	float eyeAlt = dot(-position.xyz, mat3(matrixModelView) * vec3(0, 1, 0));

	//Calculating Fog Factor For UnderWater
	fogFactor = exp2(-fogDensity * length(position)* max(waterDepth, 0) / eyeAlt) ;

	//Calculating Fog Factor For Normal Fog
	fogFactor2 = exp2(-fogDensity2 * length(position)) ;

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
