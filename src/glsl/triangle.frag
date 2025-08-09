#version 450
#extension GL_ARB_gpu_shader_fp64 : enable
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragCoord;

layout(push_constant) uniform PassData {
    int width;
    int height;
    double x;
    double y;
    double scale;
} pc;

#define UNIT double
#define MAXITERS 1024

int ProcessCoord(UNIT c_real, UNIT c_imag, int MaxIterations) {
	UNIT z_real = 0.0;
	UNIT z_imag = 0.0;

	for (int i = 0; i < MaxIterations; i++) {
		UNIT z_real_squared = z_real*z_real;
		UNIT z_imag_squared = z_imag*z_imag;

		if (z_real_squared + z_imag_squared > 4) return i;

		UNIT z_real_temp = (z_real_squared - z_imag_squared) + c_real;
		UNIT z_imag_temp = ((2 * z_real) * z_imag) + c_imag;

		z_real = z_real_temp;
		z_imag = z_imag_temp;
	}
	return MaxIterations;
}

void main() {
	UNIT c_real = ((gl_FragCoord.x / pc.width) * 4.0 - 2.5) * (float(pc.width) / float(pc.height));
	UNIT c_imag = (gl_FragCoord.y / pc.height) * 4.0 - 2.0;
	int ProcessedCoordColor = ProcessCoord(c_real, c_imag, MAXITERS);
	if (ProcessedCoordColor == MAXITERS) {
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		float intensity = pow(float(ProcessedCoordColor) / float(MAXITERS), 0.4);
		outColor = vec4(intensity, 0.0, 0.0, 1.0);
	}
}
