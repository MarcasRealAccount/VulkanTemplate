#version 460

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 projView;
} ubo;

void main() {
	vec4 worldPosition = /*ubo.model **/ inPosition;
	gl_Position = /*ubo.projView **/ worldPosition;
	outUV = inUV;
}