#version 330

// Water-related
uniform vec3 waterColor;
uniform vec3 skyColor;
uniform sampler2D texture0;
uniform samplerCube textureCubeMap;
uniform float reflectionPower;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor;			// reflection coefficient
in vec3 texCoordCubeMap;
// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;
	//This gives the water its Colour and transparancy
	outColor = mix(vec4(waterColor,0.2), vec4(skyColor,1.0), reflFactor);

	//outColor *= texture(texture0, texCoord0);

	outColor = mix(outColor * texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);


}
