#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

layout(location = 0) out vec4 gbuffer_albedo;
layout(location = 1) out vec4 gbuffer_normal;
layout(location = 2) out vec4 gbuffer_position;
layout(location = 3) out vec4 gbuffer_output;

void main() {
  gbuffer_albedo = vec4(color, 1.0);
  gbuffer_normal = vec4(normal, 1.0);
  gbuffer_position = vec4(position, 1.0);
  gbuffer_output = vec4(0.0, 0.0, 0.0, 1.0);
}
