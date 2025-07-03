#pragma once
class DefaultShader
{
	friend class Renderer;
	friend class RendererDX11;
	friend class RendererDX12;
protected:
	//Default Shader
	const char* m_defaultShaderSource = R"(
	cbuffer CameraConstants : register(b2)
	{
		float4x4 WorldToCameraTransform;
		float4x4 CameraToRenderTransform;
		float4x4 RenderToClipTransform;
	};
	
	cbuffer ModelConstants : register(b3)
	{
		float4x4 ModelToWorldTransform;
		float4 ModelColor;
	};

	struct vs_input_t
	{
		float3 modelSpacePosition : POSITION;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};
	
	struct v2p_t
	{
		float4 clipSpacePosition : SV_Position;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	Texture2D diffuseTexture : register(t0);

	SamplerState diffuseSampler : register(s0);
	
	v2p_t VertexMain(vs_input_t input)
	{
		float4 modelSpacePosition = float4(input.modelSpacePosition, 1);
		float4 worldSpacePosition = mul(ModelToWorldTransform,modelSpacePosition);
		float4 cameraSpacePosition = mul(WorldToCameraTransform, worldSpacePosition);
		float4 renderSpacePosition = mul(CameraToRenderTransform, cameraSpacePosition);
		float4 clipSpacePosition = mul(RenderToClipTransform, renderSpacePosition);
	
		v2p_t v2p;
		v2p.clipSpacePosition = clipSpacePosition;
		v2p.color = input.color;
		v2p.uv = input.uv;
		return v2p;
	}

	float4 PixelMain(v2p_t input) : SV_Target0
	{
		float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
		float4 vertexColor = input.color;
		float4 color = textureColor * vertexColor * ModelColor;
		clip(color.a - 0.01f);
		return float4(color);
	}


	)";

	const char* m_defaultShaderSourceIgnoreTextures = R"(
	cbuffer CameraConstants : register(b2)
	{
		float4x4 WorldToCameraTransform;
		float4x4 CameraToRenderTransform;
		float4x4 RenderToClipTransform;
	};
	
	cbuffer ModelConstants : register(b3)
	{
		float4x4 ModelToWorldTransform;
		float4 ModelColor;
	};

	struct vs_input_t
	{
		float3 modelSpacePosition : POSITION;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};
	
	struct v2p_t
	{
		float4 clipSpacePosition : SV_Position;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	Texture2D diffuseTexture : register(t0);

	SamplerState diffuseSampler : register(s0);
	
	v2p_t VertexMain(vs_input_t input)
	{
		float4 modelSpacePosition = float4(input.modelSpacePosition, 1);
		float4 worldSpacePosition = mul(ModelToWorldTransform,modelSpacePosition);
		float4 cameraSpacePosition = mul(WorldToCameraTransform, worldSpacePosition);
		float4 renderSpacePosition = mul(CameraToRenderTransform, cameraSpacePosition);
		float4 clipSpacePosition = mul(RenderToClipTransform, renderSpacePosition);
	
		v2p_t v2p;
		v2p.clipSpacePosition = clipSpacePosition;
		v2p.color = input.color;
		v2p.uv = input.uv;
		return v2p;
	}

	float4 PixelMain(v2p_t input) : SV_Target0
	{
		float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
		float4 vertexColor = input.color;
		float4 color = ModelColor * vertexColor;
		clip(color.a - 0.01f);
		return float4(color);
	}


	)";
};

