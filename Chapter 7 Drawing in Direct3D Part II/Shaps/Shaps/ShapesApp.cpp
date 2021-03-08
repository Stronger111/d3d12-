#include "../../../Common/d3dApp.h"
#include "../../../Common/MathHelper.h"
#include "../../../Common/UploadBuffer.h"
#include "../../../Common/GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources = 3;
//存储绘制图形所需要参数的轻量级结构体,它会随着不同的应用程序而有所差别
struct RenderItem
{
	RenderItem() = default;
	/// <summary>
	/// 描述物体的局部空间相对于世界空间的世界矩阵
	/// 他定义了物体位于世界空间中的位置,朝向以及大小
	/// </summary>
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	/// <summary>
	/// 用已更新标志(dirty flag) 来表示物体的相关数据已发生改变,这意味着我们此时需要更新常量缓冲区。这由于FrameResource 中都有一个物体常量
	/// 缓冲区,所以我们必须对每个FrameResource进行设置,从而使每个帧资源都得到更新
	/// </summary>
	int NumFramesDirty = gNumFrameResources;
	/// <summary>
	/// 该索引指向的GPU常量缓冲区对于当前渲染项中的物体常量缓冲区
	/// </summary>
	UINT ObjCBIndex = -1;
	/// <summary>
	/// 此渲染项参与绘制的几何体。注意 绘制一个几何体可能会用到多个渲染项
	/// </summary>
	MeshGeometry* Geo = nullptr;
	/// <summary>
	/// 图形拓扑 三角形
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
	/// 重载
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
	//所有渲染项的列表
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	//我们应用程序根据各渲染项的绘制目的,把它们保存在不同的向量里面,即按照不同PSO（流水线状态对象）,划分到不同的向量里面
	//根据PSO来划分渲染项,比OpenGL 先进 可以划分渲染项
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
/// 构建帧资源
/// </summary>
void ShapesApp::BuildFrameResources()
{
	for (int i=0;i<gNumFrameResources;++i)
	{
		mFrameResource.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),1,(UINT)mAllRitems.size()));
	}
}
/// <summary>
/// 更新物体常量缓冲区
/// </summary>
/// <param name="gt"></param>
void ShapesApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObejctCB = mCurrFrameResource->ObjectCB.get();
	//遍历所有的渲染项
	for (auto& e : mAllRitems)
	{
		//只要常量发生了改变就得更新常量缓冲区内的数据,而且要对每个帧资源都进行更新 NumFramesDirty多少帧脏掉了
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			currObejctCB->CopyData(e->ObjCBIndex, objConstants);

			//还需要对下一个FrameResource进行更新
			e->NumFramesDirty--;
		}
	}
}
/// <summary>
/// 更新渲染Pass cbuffer
/// </summary>
/// <param name="gt"></param>
void ShapesApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	//重点推到一下
	XMMATRIX viewProj = XMMatrixMultiply(view,proj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//XMMatrixDeterminant 行列式
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
	//常量缓冲区上传到uploadBuffer里面
	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0,mMainPassCB);
}

void ShapesApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	//根参数可能是描述符表,根描述符,或根常量
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	//创建CBV
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1,&cbvTable1);

	//根签名由一系列根参数构成
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2,slotRootParameter,0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//创建一根签名用一个单一的slot 单一的Constant Buffer
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
