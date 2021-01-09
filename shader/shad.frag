#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}