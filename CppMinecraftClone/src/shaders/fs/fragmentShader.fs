#version 330 core

uniform sampler2D textureVal;
uniform float opacity;

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(textureVal, TexCoord) * opacity;
}