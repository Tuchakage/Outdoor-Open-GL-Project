#version 330
uniform mat4 matrixView;

// Water-related
uniform vec3 waterColor;
uniform sampler2D textureBed;
uniform sampler2D textureShore;
uniform vec3 fogColour;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform float attenuation;
uniform int atton;
uniform int fogon;
uniform sampler2D textureNormal;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
// Input: Water Related
in float waterDepth;		// water depth (positive for underwater, negative for the shore)
in float fogFactor, fogFactor2;
in mat3 matrixTangent;



uniform sampler2D texture0;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2, lightPoint3, lightPoint4, lightPoint5, lightPoint6;

vec3 normalNew;
vec4 PointLight(POINT light)
{
	// Calculate Point Light (Diffuse)
	vec4 color = vec4(0, 0, 0, 0);
	//Transform the Light position with the matrix view (Multiplying part) Then subtract that with the position of the vertex, normalize it then convert the result to vec3
	//Dont forget to convert the light.position to vec4
	vec3 L = normalize(matrixView * vec4(light.position, 1)- position).xyz;
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	//Calculate Point Light (Specular)
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// Light Attenuation
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (attenuation * dist * dist);

	if (atton == 1)
	{
		return color * att;
	}
	else
	{
		return color;
	}
	
}

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
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}


void main(void) 
{

	normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0);
	normalNew = normalize(matrixTangent * normalNew);

	outColor = color;


	if (lightPoint1.on == 1)
	{
		outColor += PointLight(lightPoint1);
	} 

	if (lightPoint2.on == 1)
	{
		outColor += PointLight(lightPoint2);
	} 

	if (lightPoint3.on == 1)
	{
		outColor += PointLight(lightPoint3);
	} 

	if (lightPoint4.on == 1)
	{
		outColor += PointLight(lightPoint4);
	} 

	if (lightPoint5.on == 1)
	{
		outColor += PointLight(lightPoint5);
	} 

	if (lightPoint6.on == 1)
	{
		outColor += PointLight(lightPoint6);
	}
	
		//Contains The Colour Of The Light
	if (lightAmbient.on == 1)
	{
		outColor += AmbientLight(lightAmbient);
	}
	if (lightAmbient2.on == 1)
	{
		outColor += AmbientLight(lightAmbient2);
	}

	if (lightDir1.on == 1)
	{
		outColor += DirectionalLight(lightDir1);
	} 

	//outColor *= texture(texture0, texCoord0);
	// shoreline multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1); 
	outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);
	
	//For Water Fog
	outColor = mix(vec4(waterColor, 1), outColor, fogFactor);

	//For Normal Fog

	if (fogon == 1)
	{
		outColor = mix(vec4(fogColour, 1), outColor, fogFactor2);
	}
	
}
