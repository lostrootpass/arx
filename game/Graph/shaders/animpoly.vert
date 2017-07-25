#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in int inTexId;
layout(location = 3) in vec3 inNorm;
layout(location = 4) in vec4 inColor;
layout(location = 5) in int boneId;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec2 uv;
layout(location = 2) out int texId;
layout(location = 3) out vec3 norm;
layout(location = 4) out vec4 outColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

struct Bone
{
	mat4 rotate;
	vec4 translate;
	vec4 scale;
	int parentID;
};

layout(std140) uniform BoneData
{
    Bone bones[64];
};

uniform int numBones;

uniform vec4 modelAngle;
uniform vec4 modelOffset;

uniform vec4 linkedObjectOffset;
uniform mat4 linkedObjectMatrix;

vec3 rotateX(vec3 inVec, float angle)
{
    float c = cos(radians(angle));
    float s = sin(radians(angle));

    vec3 outVec;

    outVec.x = inVec.x;
    outVec.y = (inVec.y * c) - (inVec.z * s);
    outVec.z = (inVec.y * s) + (inVec.z * c);

    return outVec;
}

vec3 rotateY(vec3 inVec, float angle)
{
    float c = cos(radians(angle));
    float s = sin(radians(angle));

    vec3 outVec;

    outVec.x = (inVec.x * c) + (inVec.z * s);
    outVec.y = inVec.y;
    outVec.z = (inVec.z * c) - (inVec.x * s);

    return outVec;
}

vec3 rotateZ(vec3 inVec, float angle)
{
    float c = cos(radians(angle));
    float s = sin(radians(angle));

    vec3 outVec;

    outVec.x = (inVec.x * c) + (inVec.y * s);
    outVec.y = (inVec.y * c) - (inVec.x * s);
    outVec.z = inVec.z;

    return outVec;
}

vec3 animate(vec3 start, int boneId)
{
	vec3 outVec = start;
	
	Bone b = bones[boneId];

    outVec -= linkedObjectOffset.xyz;
    outVec *= mat3(linkedObjectMatrix);
	outVec *= mat3(b.rotate);
	outVec += b.translate.xyz;
	
	return outVec;
}

void main()
{
    outPos = pos;
    uv = inUV;
    texId = inTexId;
    norm = inNorm;
    outColor = inColor;

    vec3 adjPos = pos;
    if(boneId > -1 && boneId < numBones)
    {
		adjPos = animate(adjPos, boneId);
		
		{
			adjPos = rotateY(adjPos, modelAngle.y);
            adjPos = rotateX(adjPos, modelAngle.x);
            adjPos = rotateZ(adjPos, modelAngle.z);
        }

        adjPos += modelOffset.xyz;
    }

    gl_Position = proj * view * model * vec4(adjPos, 1.0);
}