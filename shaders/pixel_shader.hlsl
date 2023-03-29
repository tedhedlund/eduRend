
Texture2D texDiffuse : register(t0);

Texture2D texNormal : register (t1);

Texture2D texSpecular : register(t2);

SamplerState texSampler : register(s0);

cbuffer LightAndCameraBuffer : register(b0)
{
	float4 lightposition;
	float4 cameraposition;
}

cbuffer PhongColorAndShininessBuffer : register(b1)
{	
	float4 Ka;
	float4 Kd;
	float4 Ks;
	float shininess;
}

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
	float4 WorldPos : POSITION;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
};

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1	
	/*return float4(input.Normal*0.5+0.5, 1);*/

	// TBN
	float4 normalTexture = texNormal.Sample(texSampler,input.TexCoord) * 2 - 1;
	float3x3 TBN = transpose(float3x3(input.Tangent, input.Binormal, input.Normal));
	float3 mappedNormal = mul(TBN, normalTexture.xyz);

	// diffuse 	
	float3 norm = normalize(mappedNormal);	
	float3 lightDir = normalize(lightposition.xyz - input.WorldPos.xyz);
	float diff = max(dot(input.Normal, lightDir), 0.0);
	float4 diffuse = (diff * Kd);
	float4 color = texDiffuse.Sample(texSampler, input.TexCoord);

	if(color.a <= 0) 
	{
		color = float4(1, 1, 1, 1);
	}

	// specular
	float3 viewDir = normalize(cameraposition.xyz - input.WorldPos.xyz);
	float3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 50);
	float4 specular = (spec * Ks);
	float4 specularTexture = texSpecular.Sample(texSampler, input.TexCoord);

	return  (Ka * color) + (diffuse * color) + (specular * specularTexture);
	//return float4(specularTexture);
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
	/*return float4(input.TexCoord, 0, 1);*/

	//Other solution
	/*float3x3 TBN = transpose(float3x3(input.Tangent, input.Binormal, input.Normal));
	float4 specularTexture = texSpecular.Sample(texSampler, input.TexCoord);

	float4 normalVector = texNormal.Sample(texSampler, input.TexCoord);
	float4 diffuseColor = texDiffuse.Sample(texSampler, input.TexCoord);
	float3 newNormal = mul(TBN, normalVector.xyz * 2 - 1);

	float3 lightVector = normalize(lightposition.xyz - input.WorldPos.xyz);
	float3 viewVector = normalize(cameraposition.xyz - input.WorldPos.xyz);
	float3 reflection = normalize(reflect(-lightVector, newNormal));

	float3 A = Ka.xyz;
	float3 D = max(mul(diffuseColor.xyz, dot(lightVector, input.Normal)), 0);
	float3 S = mul(Ks.xyz, pow(max(dot(reflection, viewVector), 0), 50));
	return float4((A * diffuseColor.xyz) + (D * diffuseColor.xyz) + S * specularTexture.xyz, 1);*/

}