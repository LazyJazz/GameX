#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 frag_position;
layout(location = 1) out vec3 frag_color;
layout(location = 2) out vec2 frag_texcoord;
layout(location = 3) out vec3 frag_normal;
layout(location = 4) out vec3 frag_tangent;

layout(binding = 0, set = 0) uniform CameraData {
  mat4 view;
  mat4 proj;
}
camera_data;

layout(binding = 0, set = 1) uniform EntityData {
  mat4 model;
}
entity_data;

void main() {
  frag_position = vec3(entity_data.model * vec4(position, 1.0));
  frag_color = color;
  frag_texcoord = texcoord;
  frag_normal = normalize(
      vec3(transpose(inverse(entity_data.model)) * vec4(normal, 0.0)));
  frag_tangent = vec3(normalize(entity_data.model * vec4(tangent, 0.0)));

  gl_Position = vec4(1, -1, 1, 1) * (camera_data.proj * camera_data.view *
                                     vec4(frag_position, 1.0));
}
