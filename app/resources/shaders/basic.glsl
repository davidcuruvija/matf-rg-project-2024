//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 intensity;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform PointLight point_light;
uniform SpotLight spot_light;
uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

void main()
{
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(point_light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float distance = length(point_light.position - FragPos);
    float attenuation = 1.0 / (1.0 + point_light.linear*distance + point_light.quadratic*distance*distance);
    vec3 ambient = point_light.ambient * point_light.intensity * attenuation;
    vec3 diffuse = point_light.diffuse * diff * attenuation;
    vec3 pointResult = (ambient + diffuse) * texColor;

    vec3 spotDir = normalize(spot_light.position - FragPos);
    float theta = dot(normalize(-spotDir), normalize(spot_light.direction));
    float epsilon = spot_light.cutOff - spot_light.outerCutOff;
    float intensity = clamp((theta - spot_light.outerCutOff)/epsilon, 0.0, 1.0);
    float spotDistance = length(spot_light.position - FragPos);
    float spotAttenuation = 1.0 / (spot_light.constant + spot_light.linear * spotDistance + spot_light.quadratic * spotDistance * spotDistance);
    vec3 spotAmbient = spot_light.ambient * texColor * spotAttenuation * intensity * 2.0;
    vec3 spotDiffuse = spot_light.diffuse * max(dot(norm, spotDir), 0.0) * texColor * spotAttenuation * intensity * 2.0;
    vec3 result = pointResult + spotAmbient + spotDiffuse;
    FragColor = vec4(result, 1.0);
}
