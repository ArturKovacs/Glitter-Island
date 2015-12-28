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
	const float radius = 2;
	for (float y = -radius; y <= radius+0.01; y++) {
		aoFactor += texture(aoValue, (gl_FragCoord.xy+vec2(0, y))/screenDimensions).x*(1/(sqrt(PI*radius*radius)))*exp(-(y*y)/(2*radius*radius));
	}
	
	gl_FragColor = vec4(vec3(aoFactor), 1);
}
