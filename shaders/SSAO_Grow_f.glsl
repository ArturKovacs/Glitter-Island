#version 330

uniform sampler2D objectDepth;
uniform sampler2D aoValue;

uniform int screenWidth;
uniform int screenHeight;

const float PI = 3.1415926536;

void main()
{
	vec2 screenDimensions = vec2(screenWidth, screenHeight);
	
	float aoFactor = 0;
	const float radius = 1;
	for (float y = -radius; y <= radius+0.01; y += 1) {
		for (float x = -radius; x <= radius+0.01; x += 1) {
			aoFactor = max(texture(aoValue, (gl_FragCoord.xy+vec2(x, y))/screenDimensions).r, aoFactor);
		}
	}
	
	gl_FragColor = vec4(vec3(aoFactor), 1);
}
