#version 330

// Water-related
uniform vec3 waterColor;
uniform vec3 skyColor;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor;			// reflection coefficient

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;
	//This gives the water its Colour and transparancy
	outColor = mix(vec4(0.2), vec4(1.0), reflFactor);
}
