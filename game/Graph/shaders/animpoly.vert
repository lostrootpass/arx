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
    vec4 quat;
	vec4 translate;
	vec4 scale;
	int parentID;
};

layout(std140) uniform BoneData
{
    Bone bones[64];
};

uniform int numBones;

uniform vec4 linkedObjectOffset;
uniform mat4 linkedObjectMatrix;

vec4 QuatMultiply(vec4 q1, vec4 q2)
{
    vec4 outQuat;

    outQuat.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	outQuat.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
	outQuat.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
	outQuat.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

    return outQuat;
}

vec3 TransformVertexQuat(vec4 quat, vec3 inVec)
{
    vec3 outVec = inVec;

	float rx = inVec.x * quat.w - inVec.y * quat.z + inVec.z * quat.y;
	float ry = inVec.y * quat.w - inVec.z * quat.x + inVec.x * quat.z;
	float rz = inVec.z * quat.w - inVec.x * quat.y + inVec.y * quat.x;
	float rw = inVec.x * quat.x + inVec.y * quat.y + inVec.z * quat.z;

	outVec.x = quat.w * rx + quat.x * rw + quat.y * rz - quat.z * ry;
	outVec.y = quat.w * ry + quat.y * rw + quat.z * rx - quat.x * rz;
	outVec.z = quat.w * rz + quat.z * rw + quat.x * ry - quat.y * rx;

    return outVec;
}

mat4 MatrixFromQuat(vec4 quat)
{
    mat4 m;
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat.x + quat.x;
	y2 = quat.y + quat.y;
	z2 = quat.z + quat.z;
	xx = quat.x * x2;
	xy = quat.x * y2;
	xz = quat.x * z2;
	yy = quat.y * y2;
	yz = quat.y * z2;
	zz = quat.z * z2;
	wx = quat.w * x2;
	wy = quat.w * y2;
	wz = quat.w * z2;

	m[0][0] = 1.0 - (yy + zz);
	m[1][0] = xy - wz;
	m[2][0] = xz + wy;
	m[3][0] = 0.0;

	m[0][1] = xy + wz;
	m[1][1] = 1.0 - (xx + zz);
	m[2][1] = yz - wx;
	m[3][1] = 0.0;

	m[0][2] = xz - wy;
	m[1][2] = yz + wx;
	m[2][2] = 1.0 - (xx + yy);
	m[3][2] = 0.0;

    return m;
}

vec3 animate(vec3 start, int boneId)
{
	vec3 outVec = start;

    int boneNum = boneId;
    const int TREE_MAX_SIZE = 32;
    int tree[TREE_MAX_SIZE];
    int treeSize = -1;

    do
    {
        treeSize++;
        tree[treeSize] = boneNum;
        boneNum = bones[boneNum].parentID;
    } while(boneNum > -1 && treeSize < TREE_MAX_SIZE);

    vec4 quat = bones[0].quat;
    vec3 transVec = bones[0].translate.xyz;
    for(int i = treeSize-1; i >= 0; --i)
    {
        transVec += TransformVertexQuat(quat, bones[tree[i]].translate.xyz);
        quat = QuatMultiply(quat, bones[tree[i]].quat);
    }
    outVec *= transpose(mat3(MatrixFromQuat(quat)));
    outVec += transVec;
	
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
    adjPos -= linkedObjectOffset.xyz;
    adjPos *= mat3(linkedObjectMatrix);

    if(boneId > -1 && boneId < numBones)
    {    
		adjPos = animate(adjPos, boneId);
    }

    gl_Position = proj * view * model * vec4(adjPos, 1.0);
}