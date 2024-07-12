#version 330

#define MAX_LIGHTS 4

// Input vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec2 aCoords;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 matLight[MAX_LIGHTS];

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragColor;
out vec3 fragNormal;
out vec4 fragLightSpace[MAX_LIGHTS];
out mat3 TBN;

const float normalOffset = 0.1;

void main()
{
    // Compute binormal from vertex normal and tangent
    vec3 vertexBinormal = cross(aNormal, aTangent);
    
    // Compute fragment normal based on normal transformations
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    
    // Compute fragment position based on model transformations
    fragPosition = vec3(matModel*vec4(aPos, 1.0f));

    fragTexCoord = aCoords;
    fragNormal = normalize(normalMatrix*aNormal);
    vec3 fragTangent = normalize(normalMatrix*aTangent);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal)*fragNormal);
    vec3 fragBinormal = normalize(normalMatrix*vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    fragColor = aColor;

    for (int i = 0; i < MAX_LIGHTS; i++) {
       fragLightSpace[i] = matLight[i] * vec4(fragPosition, 1.0);
    }

    // Calculate final vertex position
    gl_Position = mvp*vec4(aPos, 1.0);
}