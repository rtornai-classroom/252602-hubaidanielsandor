#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform bool isLightSource;
uniform bool lightingEnabled;
uniform sampler2D sunTexture;

void main() {
    if (isLightSource) {
        // Textúra + Erõs Sárga szûrõ a Napnak
        vec4 texColor = texture(sunTexture, TexCoord);
        FragColor = texColor * vec4(1.5, 1.1, 0.0, 1.0); 
        return;
    }

    vec3 objectColor = vec3(1.0, 1.0, 1.0); // Fehér kockák
    
    if (!lightingEnabled) {
        FragColor = vec4(objectColor, 1.0);
        return;
    }

    float ambientStrength = 0.25; 
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0); 

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * 0.6) * lightColor; 

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}