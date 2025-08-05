#version 330 core

uniform sampler2D textureVal;
uniform float opacity;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDir;

in vec2 TexCoord;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main()
{
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 texColor = texture(textureVal, TexCoord).rgb;
    vec3 result = (ambient + diffuse) * texColor * objectColor;
    
    FragColor = vec4(result, opacity);
}