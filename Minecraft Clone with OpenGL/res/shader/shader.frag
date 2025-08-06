#version 330 core

in  vec3 FragPos;      // From vertex shader
in  vec3 Normal;       // From vertex shader
in  vec3 VertexColor;  // From vertex shader
out vec4 FragColor;

uniform vec3 lightPos;     // Position of a point-light in world space
uniform vec3 viewPos;      // Camera position (for specular)
uniform vec3 lightColor;   // e.g. vec3(1.0)

void main() {
    // ---- Ambient term ----
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // ---- Diffuse term (Lambert) ----
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(-lightPos);  // since lightPos is actually a *direction* pointing from the light source to the scene
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ---- Specular term (Phong) ----
    float specularStrength = 0.5;
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // ---- Combine ----
    vec3 result = (ambient + diffuse + specular) * VertexColor;
    FragColor = vec4(result, 1.0);
}