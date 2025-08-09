#version 450
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PassData {
    int width;
    int height;
} pc;

void main() {
	outColor = vec4(gl_FragCoord.x/pc.width, 0.0, gl_FragCoord.y/pc.width, 1.0);
}
