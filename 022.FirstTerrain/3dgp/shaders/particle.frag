#version 330

in float age;
uniform sampler2D texture0;
out vec4 outColor;
uniform int weather;
void main()
{
	if (weather == 1)
	{
		outColor = texture(texture0, gl_PointCoord);
		outColor.a = 1 - outColor.r * outColor.g * outColor.b;
		outColor.a *= 1 - age;
	}

}
