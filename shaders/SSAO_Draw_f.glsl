#version 330

uniform sampler2D objectColor;
uniform sampler2D objectDepth;

uniform sampler2D aoValue;

uniform int screenWidth;
uniform int screenHeight;

void main()
{
	vec2 screenDimensions = vec2(screenWidth, screenHeight);
	gl_FragDepth = texture(objectDepth, gl_FragCoord.xy/screenDimensions).x;
	
	float aoFactor = texture(aoValue, gl_FragCoord.xy/screenDimensions).x;
	
	gl_FragColor = vec4(texelFetch(objectColor, ivec2(gl_FragCoord.xy), 0).xyz * (1-aoFactor), 1);
}
