#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 instance_position;
layout(location = 6) in vec3 instance_color;

layout(location = 0) out vec3 frag_position;
layout(location = 1) out vec3 frag_color;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_tangent;

layout(binding = 0, set = 0, std140) uniform CameraData {
  mat4 view;
  mat4 proj;
}
camera_data;

layout(binding = 0, set = 1, std140) uniform GlobalSettings {
  float scale;
}
global_settings;

void main() {
  frag_position = position * global_settings.scale + instance_position;
  frag_color = instance_color * color;
  frag_normal = normal;
  frag_tangent = tangent;

  gl_Position = vec4(1, -1, 1, 1) * (camera_data.proj * camera_data.view *
                                     vec4(frag_position, 1.0));
}
