#version 330 core
 layout (location = 0) out vec4 FragColor;
 layout (location = 1) out vec4 BrightColor;
//out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
//    FragColor = texture(skybox, TexCoords);
    // Bloom
    float brightness = dot(vec3(FragColor), vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = FragColor;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color

    FragColor = texture(skybox, TexCoords);
}
