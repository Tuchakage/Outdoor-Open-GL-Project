#version 330

// Water-related
uniform vec3 waterColor;
uniform vec3 skyColor;
uniform sampler2D texture0;
uniform samplerCube textureCubeMap;
uniform float reflectionPower;


uniform vec3 fogColour;
uniform int fogon;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor;			// reflection coefficient
in vec3 texCoordCubeMap;
in float fogFactor;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;
	//This gives the water its Colour and transparancy
	outColor = mix(vec4(waterColor,0.2), vec4(skyColor,1.0), reflFactor);

	//outColor *= texture(texture0, texCoord0);

	outColor = mix(outColor * texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);

	//For Normal Fog
	if (fogon == 1)
	{
		outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
	}

}
