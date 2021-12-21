#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer; // "scene"
uniform sampler2D bloomBlur;

uniform bool hdr;
uniform bool bloom;

uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(hdr)
    {
        if(bloom)
            hdrColor += bloomColor; // additive blending

        // reinhard  tone mapping
         vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
         result = vec3(1.0) - exp(-hdrColor * exposure); // Eksperimentisi sa plusevima
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}

