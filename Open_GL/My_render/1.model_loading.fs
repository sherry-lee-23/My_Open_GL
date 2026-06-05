#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
};

#define NR_POINT_LIGHTS 100

uniform sampler2D texture_diffuse1;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int pointLightCount;

// ===== 先声明函数 =====
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos);

// =========================
void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);

    vec3 result = vec3(0.0);

    // ===== 方向光 =====
    vec3 lightDir = normalize(-dirLight.direction);
    float diffDir = max(dot(norm, lightDir), 0.0);

    vec3 dirLightResult =
        dirLight.ambient +
        diffDir * dirLight.diffuse;

    result += dirLightResult * color;

    // ===== 点光 =====
    for(int i = 0; i < pointLightCount; i++)
    {
        result += CalcPointLight(pointLights[i], norm, FragPos);
    }

    FragColor = vec4(result, 1.0);
}

// =========================
// 点光函数
// =========================
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos)
{
    vec3 lightVec = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightVec), 0.0);

    float distance = length(light.position - fragPos);

    float attenuation =
        1.0 / (light.constant +
               light.linear * distance +
               light.quadratic * distance * distance);

    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;

    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;

    ambient *= attenuation;
    diffuse *= attenuation;

    return ambient + diffuse;
}