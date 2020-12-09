#version 330 core

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    sampler2D diffusemap;
    sampler2D specularmap;
    sampler2D normalMap;
	int useDiffuseMap;
	int useSpecularMap;
};

struct Light {
	int type; //0-ambient, 1-directional, 2-point, 3-spot
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
	
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

uniform vec4 renderColor;
uniform vec3 viewPos; 

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
flat in int mUseNormalMap;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

out vec4 FragColor;

void main() {
    vec4 ambient;
	vec4 diffuse;
    vec4 specular;

    // ambient
    if (material.useDiffuseMap > 0) {
        ambient = light.ambient * texture(material.diffusemap, TexCoords).rgba;
    } else {
        ambient = light.ambient * material.ambient;
    }

	if (light.type > 0) {
		vec3 norm;
		if (mUseNormalMap > 0) {
		    norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space
		} else {
			norm = normalize(Normal);
		}

	    // diffuse 
		float diff;
		float distance;
		vec3 lightDir;
		if (light.type > 1) {
			if (mUseNormalMap > 0) {
			    lightDir = normalize(TangentLightPos - TangentFragPos);
			    distance = length(TangentLightPos - TangentFragPos);
			} else {
				lightDir = normalize(light.position - FragPos);
				distance = length(light.position - FragPos);
			}
		} else {
			lightDir = normalize(light.position);
//			lightDir = normalize(-light.direction);
		}
		if (mUseNormalMap > 0) {
		    diff = max(dot(lightDir, norm), 0.0);
		} else {
			diff = max(dot(norm, lightDir), 0.0);
		}
	    if (material.useDiffuseMap > 0) {
		    diffuse = light.diffuse * diff * texture(material.diffusemap, TexCoords).rgba;
	    } else {
		    diffuse = light.diffuse * (diff * material.diffuse);
	    }
	    
	    // specular
	    vec3 viewDir;
	    vec3 reflectDir;
	    float spec;
	    if (mUseNormalMap > 0) {
	    	viewDir = normalize(TangentViewPos - TangentFragPos);
	    	reflectDir = reflect(-lightDir, norm);
	    	vec3 halfwayDir = normalize(lightDir + viewDir);
	    	spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
	    } else {
	    	viewDir = normalize(viewPos - FragPos);
	    	reflectDir = reflect(-lightDir, norm);
	    	spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	    }
	    if (material.useSpecularMap > 0) {
		    specular = light.specular * spec * texture(material.specularmap, TexCoords).rgba;
	    } else {
		    specular = light.specular * (spec * material.specular);
	    }
	    
	    if (light.type > 1) {
	    	if (light.type == 3) {
	   		    // spotlight (with soft edges)
			    float theta = dot(lightDir, normalize(-light.direction)); 
			    float epsilon = (light.cutOff - light.outerCutOff);
			    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
			    diffuse *= intensity;
			    specular *= intensity;
	    	}
	
		    // attenuation
		    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
		    ambient *= attenuation;
		    diffuse *= attenuation;
		    specular *= attenuation;
	    }
    }
        
    FragColor = (ambient + diffuse + specular) * renderColor;
}
