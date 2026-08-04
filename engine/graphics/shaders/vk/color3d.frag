#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vWorldPos;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 0) uniform sampler2D normalMap;
layout(set = 2, binding = 0) uniform sampler2D metallicMap;
layout(set = 3, binding = 0) uniform sampler2D roughnessMap;
layout(set = 4, binding = 0) uniform sampler2D aoMap;

struct Light {
    ivec4 meta;
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    vec4 spotCutoff;
};
layout(std140, set = 5, binding = 0) uniform MaterialScene {
    ivec4 counts;
    vec4 globalAmbient;
    Light lights[8];
} scene;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 ambientProduct;
    vec4 diffuseProduct;
    vec4 materialSpecularShininess;
    vec4 cameraPositionFlags;
} pc;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness * roughness * roughness;
    float nh2 = max(dot(N, H), 0.0); nh2 *= nh2;
    float d = nh2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float geometrySchlickGGX(float nv, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    return nv / (nv * (1.0 - k) + k);
}

vec3 fresnelSchlick(float cosine, vec3 f0) {
    return f0 + (1.0 - f0) * pow(max(1.0 - cosine, 0.0), 5.0);
}

void main() {
	uint flags = uint(pc.cameraPositionFlags.w + 0.5);
    vec3 N = normalize(vNormal);
    if((flags & 32u) != 0u) {
        vec3 albedo = (flags & 1u) != 0u ? pow(texture(albedoMap, vUV).rgb, vec3(2.2)) : vec3(0.5);
		float metallic = (flags & 4u) != 0u ? texture(metallicMap, vUV).r : 0.0;
		float roughness = (flags & 8u) != 0u ? texture(roughnessMap, vUV).r : 0.5;
		float ao = (flags & 16u) != 0u ? texture(aoMap, vUV).r : 1.0;
		// Match OpenGL's derivative-based tangent frame for glTF normal maps.
		if((flags & 2u) != 0u) {
			vec3 tangentNormal = texture(normalMap, vUV).xyz * 2.0 - 1.0;
			vec3 q1 = dFdx(vWorldPos); vec3 q2 = dFdy(vWorldPos);
			vec2 st1 = dFdx(vUV); vec2 st2 = dFdy(vUV);
			vec3 T = normalize(q1 * st2.y - q2 * st1.y);
			vec3 B = -normalize(cross(N, T));
			N = normalize(mat3(T, B, N) * tangentNormal);
		}
		vec3 V = normalize(pc.cameraPositionFlags.xyz - vWorldPos);
		vec3 f0 = mix(vec3(0.04), albedo, metallic);
		vec3 lo = vec3(0.0);
		vec3 ambientSum = vec3(0.0);
		for(int i = 0; i < scene.counts.x; i++) {
			if((scene.counts.y & (1 << i)) == 0) continue;
			int type = scene.lights[i].meta.x;
			if(type == 0) { ambientSum += scene.lights[i].ambient.rgb; continue; }
			vec3 lightVector;
			vec3 radiance;
			if(type == 1) {
				lightVector = normalize(-scene.lights[i].direction.xyz);
				radiance = scene.lights[i].diffuse.rgb * PI;
			} else {
				vec3 delta = scene.lights[i].position.xyz - vWorldPos;
				float distanceToLight = length(delta);
				lightVector = delta / max(distanceToLight, 0.00001);
				vec3 attenuation = scene.lights[i].attenuation.xyz;
				float falloff = 1.0 / (attenuation.x + attenuation.y * distanceToLight
						+ attenuation.z * distanceToLight * distanceToLight);
				radiance = scene.lights[i].diffuse.rgb * PI * falloff;
				if(type == 3) {
					float theta = dot(lightVector, normalize(-scene.lights[i].direction.xyz));
					float epsilon = scene.lights[i].spotCutoff.x - scene.lights[i].spotCutoff.y;
					radiance *= clamp((theta - scene.lights[i].spotCutoff.y) / epsilon, 0.0, 1.0);
				}
			}
			vec3 H = normalize(V + lightVector);
			vec3 F = fresnelSchlick(max(dot(H, V), 0.0), f0);
			float ndf = distributionGGX(N, H, roughness);
			float geometry = geometrySchlickGGX(max(dot(N, V), 0.0), roughness)
					* geometrySchlickGGX(max(dot(N, lightVector), 0.0), roughness);
			vec3 specular = ndf * geometry * F
					/ (4.0 * max(dot(N, V), 0.0) * max(dot(N, lightVector), 0.0) + 0.001);
			vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
			lo += (kD * albedo / PI + specular) * radiance * max(dot(N, lightVector), 0.0);
		}
		vec3 ambientLight = ambientSum;
		if(dot(ambientLight, ambientLight) < 0.001 && scene.counts.x == 0) ambientLight = scene.globalAmbient.rgb;
		vec3 color = ambientLight * albedo * ao + lo;
        color = color / (color + vec3(1.0));
        outColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
    } else {
		if((flags & 2u) != 0u) {
			vec3 tangentNormal = texture(normalMap, vUV).xyz * 2.0 - 1.0;
			vec3 q1 = dFdx(vWorldPos); vec3 q2 = dFdy(vWorldPos);
			vec2 st1 = dFdx(vUV); vec2 st2 = dFdy(vUV);
			vec3 T = normalize(q1 * st2.y - q2 * st1.y);
			vec3 B = -normalize(cross(N, T));
			N = normalize(mat3(T, B, N) * tangentNormal);
		}
		vec4 materialAmbient = (flags & 1u) != 0u ? texture(albedoMap, vUV) : pc.ambientProduct;
		vec4 materialDiffuse = (flags & 1u) != 0u ? texture(albedoMap, vUV) : pc.diffuseProduct;
		if((flags & 1u) != 0u && materialDiffuse.a < 0.5) discard;
		vec4 materialSpecular = vec4(pc.materialSpecularShininess.rgb, 1.0);
		vec3 V = normalize(pc.cameraPositionFlags.xyz - vWorldPos);
		vec4 result = vec4(0.0);
		bool hasLight = false;
		for(int i = 0; i < scene.counts.x; i++) {
			if((scene.counts.y & (1 << i)) == 0) continue;
			Light light = scene.lights[i];
			if(light.meta.x == 0) {
				result += light.ambient * materialAmbient;
				hasLight = true;
				continue;
			}
			vec3 lightVector;
			float attenuation = 1.0;
			float intensity = 1.0;
			if(light.meta.x == 1) {
				lightVector = normalize(-light.direction.xyz);
			} else {
				vec3 delta = light.position.xyz - vWorldPos;
				float distanceToLight = length(delta);
				lightVector = delta / max(distanceToLight, 0.00001);
				attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distanceToLight
						+ light.attenuation.z * distanceToLight * distanceToLight);
				if(light.meta.x == 3) {
					float theta = dot(lightVector, normalize(-light.direction.xyz));
					float epsilon = light.spotCutoff.x - light.spotCutoff.y;
					intensity = clamp((theta - light.spotCutoff.y) / epsilon, 0.0, 1.0);
				}
			}
			float ndl = max(dot(N, lightVector), 0.0);
			vec3 reflected = reflect(-lightVector, N);
			float spec = pow(max(dot(V, reflected), 0.0), pc.materialSpecularShininess.w);
			result += light.ambient * materialAmbient * attenuation
					+ (light.diffuse * ndl * materialDiffuse
					+ light.specular * spec * materialSpecular) * attenuation * intensity;
			hasLight = true;
		}
		if(!hasLight) result = scene.globalAmbient * materialAmbient;
		outColor = result * vColor;
    }
}
