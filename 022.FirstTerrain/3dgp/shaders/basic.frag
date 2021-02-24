// FRAGMENT SHADER

#version 330
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform float attenuation;
uniform int atton;

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
uniform POINT lightPoint1, lightPoint2, lightPoint3, lightPoint4, lightPoint5, lightPoint6;

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

	if (atton == 1)
	{
		return color * att;
	}
	else
	{
		return color;
	}
	
}
void main(void) 
{

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

	outColor *= texture(texture0, texCoord0);

}
