#version 450
#extension GL_ARB_gpu_shader_fp64 : enable

//layout(location = 0) out vec2 fragCoord;
layout(push_constant) uniform PassData {
    int width;
    int height;
    double x;
    double y;
    double scale;
} pc;
vec2 positions[6] = vec2[](
	vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
	vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)
);

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	//fragCoord = (positions[gl_VertexIndex] + 1.0) * 0.5 * vec2(float(pc.width), float(pc.height));
}
