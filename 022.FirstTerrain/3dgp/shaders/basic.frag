// FRAGMENT SHADER

#version 330
uniform mat4 matrixView;



//Input Variables (From Vertex Shaders)
in vec4 color;
out vec4 outColor;
in vec4 position;
in vec3 normal;





void main(void) 
{

	outColor = color;

}
