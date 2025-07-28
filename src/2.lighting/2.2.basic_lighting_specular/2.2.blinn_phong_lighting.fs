#version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec3 ViewPos;

    // Material properties
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform float shininess;  // Controls the size of the specular highlight

    void main()
    {
        // Ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular (Blinn-Phong)
        float specularStrength = 0.5;
        vec3 viewDir = normalize(ViewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);  // Blinn-Phong uses halfway vector
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        // Combine components
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }