// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

 
layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;


out vec4 color;
out vec4 position;
out vec3 normal;



void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;
		
}
