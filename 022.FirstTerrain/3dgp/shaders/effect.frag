#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(800, 600);

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
    outColor = texture(texture0, texCoord0);
}
