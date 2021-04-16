#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(800, 600);
uniform int effecton;
uniform int effect;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

//Blur Filter
const float edgeThreshold = 0.25;
// Vignette parameters
const float RADIUS = 0.75;
const float SOFTNESS = 0.45;

// Colour definitions
const vec3 lum = vec3(0.299, 0.587, 0.114);	// B&W filter
const vec3 sepia = vec3(1.2, 1.0, 0.8); 

void main(void) 
{
    //
    if (effecton == 0)
    {        
        outColor = texture(texture0, texCoord0);
    }
    else
    {
                outColor = texture(texture0, texCoord0);
            // Vignette

            // Find centre position
            vec2 centre = (gl_FragCoord.xy / resolution.xy) - vec2(0.5);

            // Distance from the centre (between 0 and 1)
            float dist = length(centre);

            // Hermite interpolation to create smooth vignette
            dist = smoothstep(RADIUS, RADIUS-SOFTNESS, dist);

            // mix in the vignette
            outColor.rgb = mix(outColor.rgb, outColor.rgb * dist, 0.5);

            // Sepia

            // Find gray scale value using NTSC conversion weights
            float gray = dot(outColor.rgb, lum);

            // mix-in the sepia effect
            outColor.rgb = mix(outColor.rgb, vec3(gray) * sepia, 0.75);
    //Switch Between Different Effects
        if (effect == 1) // INSTAGRAM-STYLE SEPIA FILTER
        {

        }
        else if (effect == 2) //Blur Filter
        {
            const int SIZE = 5;

	        vec3 v = vec3(0, 0, 0);
	    
	        int n = 0;
	        for (int k = -SIZE; k <= SIZE; k++)
		        for (int j = -SIZE; j <= SIZE; j++)
		        {
			        v += texture(texture0, texCoord0 + vec2(k, j) / resolution).rgb;
			        n++;
		        }

	        outColor = vec4(v / n, 1);

        }
    }



}
