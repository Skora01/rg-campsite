#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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

in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in mat3 TBN;

uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;
uniform vec3 lightColor;

uniform sampler2D terrainTexture;
uniform sampler2D terrainNormal;
uniform sampler2D terrainHeight;
uniform sampler2D terrainSpecular;

uniform float height_scale;
uniform bool blinn;
uniform vec3 viewPos;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
//     float height =  texture(terrainHeight, texCoords).r;
//     vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
//     return texCoords - p;
        // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(terrainHeight, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(terrainHeight, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(terrainHeight, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(-TBN*light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
    }
    else
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    }
    // combine results
    vec3 ambient = light.ambient * vec3(texture(terrainTexture, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(terrainTexture, texCoords));
    vec3 specular = light.specular * spec *  vec3(texture(terrainSpecular, texCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
   if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
    }
    else
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    }
    // attenuation
    float distance = length(TBN*light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(terrainTexture, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(terrainTexture, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(terrainSpecular, texCoords).xxx);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
    }
    else
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    }
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(terrainTexture, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(terrainTexture, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(terrainSpecular, texCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    //parallax mapping
    vec2 texCoords = ParallaxMapping(TexCoords,  viewDir);
    if(texCoords.x > 20.0 || texCoords.y > 20.0 || texCoords. x < -20.0 || texCoords.y < -20.0)
        discard;
    vec3 norm = texture(terrainNormal, texCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir, texCoords);
    // phase 2: point lights
    result += CalcPointLight(pointLight, norm, TangentFragPos, viewDir, texCoords);
    // phase 3: spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, texCoords);

    // Bloom
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color

    FragColor = vec4(result, 1.0);
}
