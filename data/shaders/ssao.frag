#version 130
uniform sampler2D normals_and_depth;
uniform mat4 invprojm;
uniform mat4 projm;
uniform vec4 samplePoints[16];

in vec2 uv;

const float strengh = 4.;
const float radius = .4f;

#define SAMPLES 16

const float invSamples = strengh / SAMPLES;

// Found here : http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float rand(vec2 co)
{
   return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	vec4 cur = texture2D(normals_and_depth, uv);
	float curdepth = texture2D(normals_and_depth, uv).a;
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;

	// get the normal of current fragment
	vec3 norm = normalize(cur.xyz * vec3(2.0) - vec3(1.0));
	// Workaround for nvidia and skyboxes
	float len = dot(vec3(1.0), abs(cur.xyz));
	if (len < 0.2 || curdepth > 0.99) discard;
	// Make a tangent as random as possible
	vec3 randvect;
	randvect.x = rand(uv);
	randvect.y = rand(vec2(randvect.x, FragPos.z));
	randvect.z = rand(randvect.xy);
	vec3 tangent = normalize(cross(norm, randvect));
	vec3 bitangent = cross(norm, tangent);

	float bl = 0.0;

	for(int i = 0; i < SAMPLES; ++i) {
		vec3 sampleDir = samplePoints[i].x * tangent + samplePoints[i].y * bitangent + samplePoints[i].z * norm;
		sampleDir *= samplePoints[i].w;
		vec4 samplePos = FragPos + radius * vec4(sampleDir, 0.0);
		vec4 sampleProj = projm * samplePos;
		sampleProj /= sampleProj.w;

		bool isInsideTexture = (sampleProj.x > -1.) && (sampleProj.x < 1.) && (sampleProj.y > -1.) && (sampleProj.y < 1.);
		// get the depth of the occluder fragment
		float occluderFragmentDepth = texture2D(normals_and_depth, (sampleProj.xy * 0.5) + 0.5).a;
		// Position of the occluder fragment in worldSpace
		vec4 occluderPos = invprojm * vec4(sampleProj.xy, 2.0 * occluderFragmentDepth - 1.0, 1.0f);
		occluderPos /= occluderPos.w;

		bool isOccluded = isInsideTexture && (sampleProj.z > (2. * occluderFragmentDepth - 1.0)) && (distance(FragPos, occluderPos) < radius);
		bl += isOccluded ? smoothstep(radius, 0, distance(samplePos, FragPos)) : 0.;
	}

	// output the result
	float ao = 1.0 - bl * invSamples;

	gl_FragColor = vec4(ao);
}
