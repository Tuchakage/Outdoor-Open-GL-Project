#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(800, 600);
uniform int effecton;
uniform int effect;
uniform sampler2D noiseTexture; // noise texture
uniform sampler2D binocTexture; // noise texture

//Blur Filter
const float edgeThreshold = 0.25;
// Vignette parameters
const float RADIUS = 0.75;
const float SOFTNESS = 0.45;

// Colour definitions
const vec3 lum = vec3(0.299, 0.587, 0.114);	// B&W filter
const vec3 sepia = vec3(1.2, 1.0, 0.8); 

//Kernel Effect
const float offset = 1.0 / 300.0;  

//Night Vision Filter
uniform float elapsedTime; // seconds for noise effect
 
float contrast = 0.5;
uniform float intensityAdjust; // = 1;
uniform float noiseAmplification; // 1
uniform float bufferAmplication; // 1
// Output Variable (sent down through the Pipeline)
out vec4 outColor;
void main(void) 
{

    if (effecton == 0)
    {        
        // If The User Hasn't Pressed The Button To Turn On Effects Then No effects will appear
        outColor = texture(texture0, texCoord0);
    }
    else
    {

        //Switch Between Different Effects
        if (effect == 1) // INSTAGRAM-STYLE SEPIA FILTER
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
        else if (effect == 3 ) //Kernel Effect
        {
            vec2 offsets[9] = vec2[](
                vec2(-offset,  offset), // top-left
                vec2( 0.0f,    offset), // top-center
                vec2( offset,  offset), // top-right
                vec2(-offset,  0.0f),   // center-left
                vec2( 0.0f,    0.0f),   // center-center
                vec2( offset,  0.0f),   // center-right
                vec2(-offset, -offset), // bottom-left
                vec2( 0.0f,   -offset), // bottom-center
                vec2( offset, -offset)  // bottom-right    
            );
    
            float kernel[9] = float[](
                -1, -1, -1,
                -1,  9, -1,
                -1, -1, -1
             );
    
            vec3 sampleTex[9];
            for(int i = 0; i < 9; i++)
            {
                sampleTex[i] = vec3(texture(texture0, texCoord0.st + offsets[i]));
            }
            vec3 col = vec3(0.0);
            for(int i = 0; i < 9; i++)
                col += sampleTex[i] * kernel[i];
    
            outColor = vec4(col, 1.0);
        }

        else if (effect == 4) // EDGE DETECTION FILTER
        {
        	float s00 = dot(lum, texture(texture0, texCoord0 + vec2(-1,  1) / resolution).rgb);
	        float s01 = dot(lum, texture(texture0, texCoord0 + vec2( 0,  1) / resolution).rgb);
	        float s02 = dot(lum, texture(texture0, texCoord0 + vec2( 1,  1) / resolution).rgb);
	        float s10 = dot(lum, texture(texture0, texCoord0 + vec2(-1,  0) / resolution).rgb);
	        float s12 = dot(lum, texture(texture0, texCoord0 + vec2( 1,  0) / resolution).rgb);
	        float s20 = dot(lum, texture(texture0, texCoord0 + vec2(-1, -1) / resolution).rgb);
	        float s21 = dot(lum, texture(texture0, texCoord0 + vec2( 0, -1) / resolution).rgb);
	        float s22 = dot(lum, texture(texture0, texCoord0 + vec2( 1, -1) / resolution).rgb);

	        float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
	        float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

	        float s = sx *sx + sy * sy;

	        if (s > edgeThreshold)
		        outColor = vec4(1.0);
	        else
		        outColor = vec4(0.0, 0.0, 0.0, 1.0);

        }
        else if (effect == 5) // Night Vision Goggles
        {
            // uv coord for noise texture and swirl effect
            vec2 uv;
            uv.x = 0.35*sin(elapsedTime*50.0);
            uv.y = 0.35*cos(elapsedTime*50.0);           
 
            // noise texture color + rotation by time
            vec3 noise = texture(noiseTexture, texCoord0.st + uv).rgb * noiseAmplification;
 
            // goggle mask
            vec3 goggleColor = texture(binocTexture, texCoord0.st).rgb;
 
            // frame buffer + litle swirl
            vec3 sceneColor = texture(texture0, texCoord0.st + (noise.xy*0.005)).rgb * bufferAmplication;      
 
            //color intensity - color dominant
            const vec3 lumvec = vec3(0.30, 0.59, 0.11);
            float intentisy = dot(lumvec,sceneColor) ;
 
            // adjust contrast - 0...1
            intentisy = clamp(contrast * (intentisy - 0.5) + 0.5, 0.0, 1.0);
 
            // final green result 0...1
            float green = clamp(intentisy / 0.59, 0.0, 1.0) * intensityAdjust;
 
            // vision color - getting green max
            vec3 visionColor = vec3(0,green,0);//vec3(0.1, 0.95, 0.2); 
 
            // final color
            outColor = (vec4(sceneColor, 1) + (vec4(noise, 1)*0.2)) * vec4(visionColor, 1) * vec4(goggleColor, 1);
        }
    }



}
