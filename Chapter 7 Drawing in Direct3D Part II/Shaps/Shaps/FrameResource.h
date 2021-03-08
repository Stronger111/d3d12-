#pragma once

#include "../../../Common/d3dUtil.h"
#include "../../../Common/MathHelper.h"
#include "../../../Common/UploadBuffer.h"
/// <summary>
/// CBuffer constants Buffer
/// </summary>
struct ObjectConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView= MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj= MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = {0.0f,0.0f,0.0f};
	//??
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = {0.0f,0.0f};
	DirectX::XMFLOAT2 InvRenderTargetSize = {0.0f,0.0f};
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

//����CPUΪ����ÿ֡�����б�����Ҫ����Դ
//���е����ݽ����������,��ȡ����ʵ�ʻ�������Ҫ����Դ
struct FrameResource
{
public:
	FrameResource(ID3D12Device* device,UINT passCount,UINT objectCount);
	/// <summary>
	/// ɾ��Ĭ�ϵĿ������캯��,�Ȳ��ܽ���Ĭ�Ͽ���
	/// </summary>
	/// <param name="rhs"></param>
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	//��GPU��������������������ص�����֮ǰ,���ǲ��ܶ�����������
	//����ÿһ֡��Ҫ�������Լ������������
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	//��GPUִ�������ô˳�������������֮ǰ,���ǲ��ܶ������и���
	//���ÿһ֡��Ҫ�������Լ��ĳ���������
	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	//ͨ��Fenceֵ�������ǵ���Fence��,��ʹ���ǿ��Լ�⵽GPU�Ƿ���ʹ����Щ֡��Դ
};