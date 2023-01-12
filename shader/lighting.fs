#version 330 core

out vec4 fragColor;

in vec3 normal;
in vec2 texCoord;
in vec3 position;

uniform vec3 viewPos;

struct Light {
#if 0// point light
	vec3 position;
	vec3 attenuation;// F attenuation = 1.0 / (kconst + klinear*d + kquadratic*d*d)
#elif 0// direction light
	vec3 direction;
#elif 1// spot light
	vec3 position;
	vec3 attenuation;// F attenuation = 1.0 / (kconst + klinear*d + kquadratic*d*d)
	vec3 direction;
	vec2 cutoff;
#endif
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light;

struct Material {
	//vec3 ambient;
	//vec3 diffuse;
	//vec3 specular;
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

void main()
{
	//vec3 ambient = material.ambient * light.ambient;
	vec3 texColor = texture2D(material.diffuse, texCoord).xyz;
	vec3 ambient = texColor * light.ambient;

#if 1// point | spot light
	//vec3 lightDir = normalize(light.position - position);
	float dist = length(light.position - position);
	vec3 distPoly = vec3(1.0f, dist, dist*dist);
	float attenuation = 1.0 / dot(distPoly, light.attenuation);
	vec3 lightDir = (light.position - position) / dist;
#elif 1// direction light
	vec3 lightDir = normalize(-light.direction);
#endif

#if 0// point | direction light
	vec3 pixelNorm = normalize(normal);
	float diff = max(dot(pixelNorm, lightDir), 0.0);
	//vec3 diffuse = diff * material.diffuse * light.diffuse;
	vec3 diffuse = diff * texColor * light.diffuse;

	vec3 viewDir = normalize(viewPos - position);
	vec3 reflectDir = reflect(-lightDir, pixelNorm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	//vec3 specular = spec * material.specular * light.specular;
	vec3 specColor = texture2D(material.specular, texCoord).xyz;
	vec3 specular = spec * specColor * light.specular;
	
#if 1// point light
	vec3 result = (ambient + diffuse + specular) * attenuation;
#elif 1// direction light
	vec3 result = ambient + diffuse + specular;
#endif

#elif 1// spot light
	vec3 result = ambient;
	float theta = dot(lightDir, normalize(-light.direction));

	//if (theta > light.cutoff) {
	float intensity = clamp((theta - light.cutoff[1]) / (light.cutoff[0] - light.cutoff[1]), 0.0, 1.0);
	if (intensity > 0.0f) {
		vec3 pixelNorm = normalize(normal);
		float diff = max(dot(pixelNorm, lightDir), 0.0);
		vec3 diffuse = diff * texColor * light.diffuse;

		vec3 specColor = texture2D(material.specular, texCoord).xyz;
		vec3 viewDir = normalize(viewPos - position);
		vec3 reflectDir = reflect(-lightDir, pixelNorm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = spec * specColor * light.specular;

		//result += diffuse + specular;
		result += (diffuse + specular) * intensity;
	}
	result *= attenuation;
#endif
	fragColor = vec4(result, 1.0);
}
