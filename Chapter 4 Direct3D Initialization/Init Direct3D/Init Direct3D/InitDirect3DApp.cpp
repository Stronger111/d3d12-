#include <DirectXColors.h>
#include "../../../Common/d3dApp.h"

using namespace DirectX;

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);

	~InitDirect3DApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	//为调试版本开启运行时内存检测,方便监督内存泄露的情况
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) | defined(_DEBUG)
	try
	{
		InitDirect3DApp theApp(hInstance);
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
//继承父类的构造函数
InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) :D3DApp(hInstance)
{

}

InitDirect3DApp ::~InitDirect3DApp()
{

}

bool InitDirect3DApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::Update(const GameTimer& gt)
{

}

void InitDirect3DApp::Draw(const GameTimer& gt)
{
	//重复使用记录命令相关的内存
	//只有当与GPU关联的命令列表执行完成时,我们才能将其重置
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	//在通过ExecuteCommandList 方法将某个命令 列表加入命令队列后,我们便可重置列表。以此来复用列表及其内存
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	//对资源的状态进行切换，将资源从呈现状态转换为渲染目标状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//设置视口和裁剪矩形。他们需要随着命令列表的重置而重置
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	//清除后台缓冲区和深度缓冲区
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	//指定将要渲染的缓冲区
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//再次对资源状态进行切换,将资源从渲染状态改为呈现状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//完成命令的记录
	ThrowIfFailed(mCommandList->Close());

	//将待执行的命令列表加入命令队列
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };

	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	//交换后台缓冲区和前台缓冲区
	ThrowIfFailed(mSwapChain->Present(0, 0));

	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	//
	FlushCommandQueue();
}
