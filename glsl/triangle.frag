#version 450
layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(gl_FragCoord.x/800, 0.0, gl_FragCoord.y/800, 1.0);
}
