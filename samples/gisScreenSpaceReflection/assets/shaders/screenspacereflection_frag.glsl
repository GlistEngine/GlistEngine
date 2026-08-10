#if GLES
#version 300 es
precision highp float;
precision highp int;
#else
#version 330 core
#endif

in vec3 viewpos;
in vec3 viewnormal;
in vec3 worldpos;
in vec2 texcoord;

out vec4 fragcolor;

uniform sampler2D scenecolor;
uniform sampler2D scenedepth;
uniform samplerCube skymap;
uniform sampler2D reflectionbuffer;
uniform sampler2D roughnessMap;
uniform int hasRoughnessMap;

uniform mat4 projection;
uniform mat4 invview;
uniform mat4 previousviewprojection;
uniform vec2 screensize;
uniform float nearclip;
uniform float farclip;

uniform float reflectivity;
uniform float fresnelbias;
uniform float fresnelpower;
uniform vec3 fallbackcolor;
uniform float ditherphase;

uniform int sspass;
uniform int validhistory;
uniform int hasskymap;

const int maxsteps = 100;
const int minmisssteps = 6;
const int binarysteps = 8;
const float initialstep = 0.02;
const float stepgrowth = 1.035;
const float normalbias = 0.02;
const float maxhitthickness = 2.0;
const float thicknessscale = 2.0;
const float discontinuityscale = 6.0;
const float edgefadedist = 0.12;
const float historyweight = 0.9;
const float maxmotionpixels = 32.0;
const float maxroughnessblur = 6.0;
const float missfeatherdistance = 1.0;

float linearizedepth(float d) {
	return nearclip * farclip / (farclip - d * (farclip - nearclip));
}

vec2 viewtoscreenuv(vec3 pos, out bool behindcamera) {
	vec4 clip = projection * vec4(pos, 1.0);
	behindcamera = clip.w <= 0.0;
	clip.xyz /= clip.w;
	return clip.xy * 0.5 + 0.5;
}

float interleavedgradientnoise(vec2 pos) {
	return fract(52.9829189 * fract(dot(pos, vec2(0.06711056, 0.00583715))));
}

vec3 sampleblurredhit(vec2 uv, float blurscale) {
	float lod = log2(max(blurscale, 1.0));
	vec2 texel = (1.0 / screensize) * blurscale;
	vec3 sum = textureLod(scenecolor, uv, lod).rgb * 4.0;
	sum += textureLod(scenecolor, uv + texel * vec2(-1.0, -1.0), lod).rgb;
	sum += textureLod(scenecolor, uv + texel * vec2(1.0, -1.0), lod).rgb;
	sum += textureLod(scenecolor, uv + texel * vec2(-1.0, 1.0), lod).rgb;
	sum += textureLod(scenecolor, uv + texel * vec2(1.0, 1.0), lod).rgb;
	return sum / 8.0;
}

bool marchray(vec3 rayorigin, vec3 raydir, float jitter, out vec2 hituv, out vec2 nearmissuv, out float missdistance) {
	vec3 raypos = rayorigin;
	float stepsize = initialstep * pow(stepgrowth, jitter);
	float prevscenelinearz = 0.0;
	bool hasprevsample = false;
	nearmissuv = vec2(-1.0);
	missdistance = missfeatherdistance;

	for(int i = 0; i < maxsteps; i++) {
		vec3 prevpos = raypos;
		raypos += raydir * stepsize;

		bool behindcamera;
		vec2 uv = viewtoscreenuv(raypos, behindcamera);
		if(behindcamera || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
			hituv = uv;
			return false;
		}

		float scenelinearz = -linearizedepth(texture(scenedepth, uv).r);
		float localthickness = min(maxhitthickness, stepsize * thicknessscale);
		bool discontinuity = hasprevsample && abs(scenelinearz - prevscenelinearz) > localthickness * discontinuityscale;
		float depthdelta = raypos.z - scenelinearz;
		if(!discontinuity && depthdelta <= 0.0 && depthdelta > -localthickness) {
			vec3 lo = prevpos;
			vec3 hi = raypos;
			for(int j = 0; j < binarysteps; j++) {
				vec3 mid = (lo + hi) * 0.5;
				bool midbehindcamera;
				vec2 miduv = viewtoscreenuv(mid, midbehindcamera);
				float midscenelinearz = -linearizedepth(texture(scenedepth, miduv).r);
				if(!midbehindcamera && mid.z <= midscenelinearz) {
					hi = mid;
				} else {
					lo = mid;
				}
			}
			bool finalbehindcamera;
			hituv = viewtoscreenuv(hi, finalbehindcamera);
			return !finalbehindcamera;
		}

		if(!discontinuity && i >= minmisssteps && depthdelta > 0.0 && depthdelta < missdistance) {
			missdistance = depthdelta;
			nearmissuv = uv;
		}

		prevscenelinearz = scenelinearz;
		hasprevsample = true;
		stepsize *= stepgrowth;
	}

	hituv = vec2(-1.0);
	return false;
}

vec4 computereflection() {
	float vnormallen = length(viewnormal);
	vec3 vnormal = vnormallen > 0.0001 ? viewnormal / vnormallen : vec3(0.0, 1.0, 0.0);
	if(dot(vnormal, viewpos) > 0.0) vnormal = -vnormal;

	vec3 viewdir = normalize(viewpos);
	vec3 raydir = reflect(viewdir, vnormal);

	float dither = fract(interleavedgradientnoise(gl_FragCoord.xy) + ditherphase);
	float dynamicbias = normalbias * (1.0 + length(viewpos) * 0.05);
	vec3 rayorigin = viewpos + vnormal * dynamicbias;

	vec2 hituv;
	vec2 nearmissuv;
	float missdistance;
	bool hit = marchray(rayorigin, raydir, dither, hituv, nearmissuv, missdistance);

	float ndotv = clamp(dot(-viewdir, vnormal), 0.0, 1.0);
	float fresnel = fresnelbias + (1.0 - fresnelbias) * pow(1.0 - ndotv, fresnelpower);
	float roughnessvalue = hasRoughnessMap == 1 ? texture(roughnessMap, texcoord).r : 0.0;
	float grazingblur = (1.0 - ndotv) * (1.0 - ndotv) * maxroughnessblur;
	float blurscale = 1.0 + roughnessvalue * maxroughnessblur + grazingblur;

	vec3 reflectioncolor;
	float reflectionamount;
	if(hit) {
		reflectioncolor = sampleblurredhit(hituv, blurscale);
		vec2 edgedist = min(hituv, 1.0 - hituv);
		float edgefade = clamp(min(edgedist.x, edgedist.y) / edgefadedist, 0.0, 1.0);
		reflectionamount = reflectivity * mix(1.0, fresnel, 0.5) * edgefade;
	} else {
		vec3 misscolor;
		if(hasskymap == 1) {
			vec3 worldraydir = normalize(mat3(invview) * raydir);
			misscolor = texture(skymap, worldraydir).rgb;
		} else {
			misscolor = fallbackcolor;
		}
		if(nearmissuv.x >= 0.0) {
			float missfeather = clamp(1.0 - missdistance / missfeatherdistance, 0.0, 1.0);
			misscolor = mix(misscolor, sampleblurredhit(nearmissuv, blurscale), missfeather);
		}
		reflectioncolor = misscolor;
		reflectionamount = reflectivity * fresnel;
	}

	return vec4(reflectioncolor, reflectionamount);
}

void main() {
	if(sspass == 1) {
		vec2 screenuv = gl_FragCoord.xy / screensize;
		vec3 basecolor = texture(scenecolor, screenuv).rgb;
		vec4 accumulated = texture(reflectionbuffer, screenuv);
		fragcolor = vec4(mix(basecolor, accumulated.rgb, clamp(accumulated.a, 0.0, 1.0)), 1.0);
		return;
	}

	vec4 fresh = computereflection();
	vec4 accumulated = fresh;

	if(validhistory == 1) {
		vec4 prevclip = previousviewprojection * vec4(worldpos, 1.0);
		if(prevclip.w > 0.0) {
			vec2 prevuv = (prevclip.xy / prevclip.w) * 0.5 + 0.5;
			if(prevuv.x >= 0.0 && prevuv.x <= 1.0 && prevuv.y >= 0.0 && prevuv.y <= 1.0) {
				vec2 currentuv = gl_FragCoord.xy / screensize;
				float motionpixels = length((prevuv - currentuv) * screensize);
				float motionfactor = clamp(motionpixels / maxmotionpixels, 0.0, 1.0);
				float adaptiveweight = historyweight * (1.0 - motionfactor);
				accumulated = mix(fresh, texture(reflectionbuffer, prevuv), adaptiveweight);
			}
		}
	}

	fragcolor = accumulated;
}
