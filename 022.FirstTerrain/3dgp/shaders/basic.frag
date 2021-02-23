// FRAGMENT SHADER

#version 330
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform float attenuation;

//Input Variables (From Vertex Shaders)
in vec4 color;
out vec4 outColor;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

uniform sampler2D texture0;

struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightpoint2;

vec4 PointLight(POINT light)
{
	// Calculate Point Light (Diffuse)
	vec4 color = vec4(0, 0, 0, 0);
	//Transform the Light position with the matrix view (Multiplying part) Then subtract that with the position of the vertex, normalize it then convert the result to vec3
	//Dont forget to convert the light.position to vec4
	vec3 L = normalize(matrixView * vec4(light.position, 1)- position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	//Calculate Point Light (Specular)
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// Light Attenuation
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (attenuation * dist * dist);

	return color * att;
}
void main(void) 
{

	outColor = color;
	//Atemmpt At Textures/ Skybox
	//outColor *= texture(texture0, texCoord0);
	if (lightPoint1.on == 1)
	{
		outColor += PointLight(lightPoint1);
	} 

}
