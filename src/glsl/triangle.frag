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
		UNIT centered_x = gl_FragCoord.x - UNIT(pc.width) * 0.5;
		UNIT centered_y = gl_FragCoord.y - UNIT(pc.height) * 0.5;

		UNIT zoomed_x = centered_x / UNIT(pc.scale);
		UNIT zoomed_y = centered_y / UNIT(pc.scale);

		UNIT final_x = zoomed_x - UNIT(pc.x);
		UNIT final_y = zoomed_y - UNIT(pc.y);

		UNIT c_real = (final_x / UNIT(pc.width)) * 4.0;
		UNIT c_imag = (final_y / UNIT(pc.height)) * 4.0;

		c_real *= UNIT(pc.width) / UNIT(pc.height);

	float iter = float(ProcessCoord(c_real, c_imag, MAXITERS));
	if (iter == MAXITERS) {
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		float r2 = max(float(c_real*c_real + c_imag*c_imag), 1e-10);
		float log_zn = log(r2) * 0.5;
		float nu = log(log_zn / log(float(2.0))) / log(2.0);
		iter = iter + 1.0 - log_zn;  // smooth iteration count
		float intensity = pow(iter / float(MAXITERS), 0.4);
		outColor = vec4(intensity, 0.0, 0.0, 1.0);
	}
}
