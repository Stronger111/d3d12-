#include "../../../Common/d3dApp.h"
#include "../../../Common/MathHelper.h"
#include "../../../Common/UploadBuffer.h"
#include "../../../Common/GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources = 3;
//�洢����ͼ������Ҫ�������������ṹ��,�������Ų�ͬ��Ӧ�ó�����������
struct RenderItem
{
	RenderItem() = default;
	/// <summary>
	/// ��������ľֲ��ռ����������ռ���������
	/// ������������λ������ռ��е�λ��,�����Լ���С
	/// </summary>
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	/// <summary>
	/// ���Ѹ��±�־(dirty flag) ����ʾ�������������ѷ����ı�,����ζ�����Ǵ�ʱ��Ҫ���³�����������������FrameResource �ж���һ�����峣��
	/// ������,�������Ǳ����ÿ��FrameResource��������,�Ӷ�ʹÿ��֡��Դ���õ�����
	/// </summary>
	int NumFramesDirty = gNumFrameResources;
	/// <summary>
	/// ������ָ���GPU�������������ڵ�ǰ��Ⱦ���е����峣��������
	/// </summary>
	UINT ObjCBIndex = -1;
	/// <summary>
	/// ����Ⱦ�������Ƶļ����塣ע�� ����һ����������ܻ��õ������Ⱦ��
	/// </summary>
	MeshGeometry* Geo = nullptr;
	/// <summary>
	/// ͼ������ ������
	/// </summary>
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

};

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	ShapesApp(const ShapesApp& rhs)=delete;
	ShapesApp& operator=(const ShapesApp& rhs) = delete;
	~ShapesApp();
	/// <summary>
	/// ����
	/// </summary>
	/// <returns></returns>
	virtual bool Initialize()override;
private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	//void DrawRenderItems(ID3D12GraphicsCommandList* cmdList,const std::vector<>);
private:
	std::vector<std::unique_ptr<FrameResource>> mFrameResource;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	//������Ⱦ����б�
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	//����Ӧ�ó�����ݸ���Ⱦ��Ļ���Ŀ��,�����Ǳ����ڲ�ͬ����������,�����ղ�ͬPSO����ˮ��״̬����,���ֵ���ͬ����������
	//����PSO��������Ⱦ��,��OpenGL �Ƚ� ���Ի�����Ⱦ��
	std::vector<RenderItem*> mOpaqueRitems;
	std::vector<RenderItem*> mTransparentRitems;

	PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = {0.0f,0.0f,0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		ShapesApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

ShapesApp::~ShapesApp()
{
	if (md3dDevice!= nullptr)
	{
		FlushCommandQueue();
	}
}
/// <summary>
/// ����֡��Դ
/// </summary>
void ShapesApp::BuildFrameResources()
{
	for (int i=0;i<gNumFrameResources;++i)
	{
		mFrameResource.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),1,(UINT)mAllRitems.size()));
	}
}
/// <summary>
/// �������峣��������
/// </summary>
/// <param name="gt"></param>
void ShapesApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObejctCB = mCurrFrameResource->ObjectCB.get();
	//�������е���Ⱦ��
	for (auto& e : mAllRitems)
	{
		//ֻҪ���������˸ı�͵ø��³����������ڵ�����,����Ҫ��ÿ��֡��Դ�����и��� NumFramesDirty����֡�����
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			currObejctCB->CopyData(e->ObjCBIndex, objConstants);

			//����Ҫ����һ��FrameResource���и���
			e->NumFramesDirty--;
		}
	}
}
/// <summary>
/// ������ȾPass cbuffer
/// </summary>
/// <param name="gt"></param>
void ShapesApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	//�ص��Ƶ�һ��
	XMMATRIX viewProj = XMMatrixMultiply(view,proj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//XMMatrixDeterminant ����ʽ
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth,(float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;

	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	//�����������ϴ���uploadBuffer����
	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0,mMainPassCB);
}

void ShapesApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	//��������������������,��������,�������
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	//����CBV
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1,&cbvTable1);

	//��ǩ����һϵ�и���������
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2,slotRootParameter,0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//����һ��ǩ����һ����һ��slot ��һ��Constant Buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,D3D_ROOT_SIGNATURE_VERSION_1,serializedRootSig.GetAddressOf(),errorBlob.GetAddressOf());
	if (errorBlob!=nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0,serializedRootSig->GetBufferPointer(),serializedRootSig->GetBufferSize(),IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}
