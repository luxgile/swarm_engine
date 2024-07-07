#version 330

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

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragColor;
out vec3 fragNormal;
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

    fragTexCoord = aCoords*2.0;
    fragNormal = normalize(normalMatrix*aNormal);
    vec3 fragTangent = normalize(normalMatrix*aTangent);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal)*fragNormal);
    vec3 fragBinormal = normalize(normalMatrix*vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    fragColor = aColor;

    // Calculate final vertex position
    gl_Position = mvp*vec4(aPos, 1.0);
}