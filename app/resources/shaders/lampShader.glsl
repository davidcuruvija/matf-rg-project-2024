//#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 FragPosWorld;

void main() {
    TexCoords = aTexCoords;
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPosWorld = worldPos.xyz;
    gl_Position = projection * view * worldPos;
}
//#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPosWorld;

uniform vec3 lightIntensity;
uniform sampler2D texture_diffuse1;
uniform float lower;
uniform float upper;
uniform float margin;

void main() {
    vec3 baseColor = texture(texture_diffuse1, TexCoords).rgb;
    float fadeLower = smoothstep(lower - margin, lower + margin, FragPosWorld.y);
    float fadeUpper = smoothstep(upper + margin, upper - margin, FragPosWorld.y);
    float factor = fadeLower * fadeUpper;
    vec3 finalColor = mix(baseColor, baseColor * lightIntensity, factor);
    FragColor = vec4(finalColor, 1.0);
}
