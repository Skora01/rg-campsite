#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

struct DirLight {
    vec3 direction;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;
};


struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


out vec2 TexCoords;
out vec3 FragPos;
out mat3 TBN;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec3 dirDir;
out vec3 pointPos;
out vec3 spotPos;
out vec3 spotDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLight;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;

    dirDir = TBN * dirLight.direction;
    pointPos = TBN * pointLight.position;
    spotPos = TBN * spotLight.position;
    spotDir = TBN * spotLight.direction;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

