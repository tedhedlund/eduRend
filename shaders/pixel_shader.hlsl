
Texture2D texDiffuse : register(t0);

SamplerState texSampler : register(s0);

cbuffer LightAndCameraBuffer : register(b0)
{
	float4 lightposition;
	float4 cameraposition;
}

cbuffer PhongColorAndShininessBuffer : register(b1)
{
	/*float4 colorandshine;*/ //ambient, diffuse, specular, shininess
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
};

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1	
	/*return float4(input.Normal*0.5+0.5, 1);*/

	////ambient
	//float ambientStrenght = 0.1f;
	//float3 ambient = ambientStrenght * float3(1, 1, 1) * Ka;

	////diffuse
	//float3 norm = normalize(input.Normal);
	//float3 lightDir = normalize(lightposition.xyz - input.Pos.xyz);
	//float diff = max(dot(norm, lightDir), 0.0);
	//float3 diffuse = Kd * diff * float3(1, 1, 1);

	////specular
	//float specularStrength = 0.5f;
	//float3 viewDir = normalize(cameraposition.xyz - input.Pos.xyz);
	//float3 reflectDir = reflect(-lightDir, norm);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	//float3 specular = specularStrength * spec * float3(1,1,1) * Ks;
	//
	//float3 results = (ambient + diffuse + specular);

	//return float4(results, 1);

	
	
	// diffuse 
	float3 norm = normalize(input.Normal);
	float3 lightDir = normalize(lightposition.xyz - input.WorldPos.xyz);
	float diff = max(dot(norm, lightDir), 0.0);
	float4 diffuse = (diff * Kd);
	float4 color = texDiffuse.Sample(texSampler, input.TexCoord);

	if(color.a <= 0) 
	{
		color = float4(1, 1, 1, 1);
	}

	// specular
	float3 viewDir = normalize(cameraposition.xyz - input.WorldPos.xyz);
	float3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	float4 specular = (spec * Ks);

	return  (Ka * color) + (diffuse * color) + specular;


	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
	/*return float4(input.TexCoord, 0, 1);*/

}