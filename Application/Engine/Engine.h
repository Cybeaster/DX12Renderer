#pragma once
#include "DirectX/FrameResource.h"
#include "DirectX/RenderItem/RenderItem.h"
#include "DirectX/ShaderTypes.h"
#include "ExitHelper.h"
#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Filters/Blur/BlurFilter.h"
#include "Filters/SobelFilter/SobelFilter.h"
#include "GraphicsPipelineManager/GraphicsPipelineManager.h"
#include "MaterialManager/MaterialManager.h"
#include "MeshGenerator/MeshGenerator.h"
#include "RenderGraph/Graph/RenderGraph.h"
#include "RenderTarget/CubeMap/DynamicCubeMap/DynamicCubeMapTarget.h"
#include "RenderTarget/RenderTarget.h"
#include "ShaderCompiler/Compiler.h"
#include "TextureManager/TextureManager.h"
#include "UI/UIManager/UiManager.h"

#include <dxgi1_6.h>

#include <map>

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	DECLARE_DELEGATE(SOnFrameResourceChanged);

	using TWindowPtr = OWindow*;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;
	using TSceneGeometryMap = std::unordered_map<string, unique_ptr<SMeshGeometry>>;
	using TRenderLayer = map<string, vector<ORenderItem*>>;
	vector<unique_ptr<SFrameResource>> FrameResources;
	SFrameResource* CurrentFrameResources = nullptr;
	UINT CurrentFrameResourceIndex = 0;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	static OEngine* Get()
	{
		if (Engine == nullptr)
		{
			Engine = new OEngine();
		}
		return Engine;
	}

	virtual ~OEngine();

	virtual bool Initialize();

private:
	void InitManagers();
	void PostInitialize();
	void InitUIManager();
	void InitCompiler();
	void InitPipelineManager();

public:
	void SetFogColor(DirectX::XMFLOAT4 Color);
	void SetFogStart(float Start);
	void SetFogRange(float Range);
	OWindow* GetWindow() const;
	ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU() const;

	int InitTests(shared_ptr<class OTest> Test);
	void PostTestInit();
	void DestroyWindow();
	void OnWindowDestroyed();
	bool IsTearingSupported() const;

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE) const;

	OCommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);

	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	void OnEnd(shared_ptr<OTest> Test) const;

	void TryRebuildFrameResource();
	void OnPreRender();
	void PrepareRenderTarget(ORenderTargetBase* RenderTarget);

	void PrepareRenderTarget();
	void Draw(UpdateEventArgs& Args);
	void Render(UpdateEventArgs& Args);
	void Update(UpdateEventArgs& Args);
	void OnUpdate(UpdateEventArgs& Args);
	void OnPostRender();
	void PostProcess(HWND Handler);
	void DrawCompositeShader(CD3DX12_GPU_DESCRIPTOR_HANDLE Input);
	void OnKeyPressed(KeyEventArgs& Args);
	void OnKeyReleased(KeyEventArgs& Args);
	void OnMouseMoved(class MouseMotionEventArgs& Args);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResizeRequest(HWND& WindowHandle);
	void OnUpdateWindowSize(ResizeEventArgs& Args);
	void SetWindowViewport();
	bool CheckTearingSupport();

	void CreateWindow();

	void CheckMSAAQualitySupport();

	bool GetMSAAState(UINT& Quality) const;

	vector<ORenderItem*>& GetRenderItems(const string& Type);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetOpaquePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetAlphaTestedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTransparentPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetShadowPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetReflectedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetMirrorPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDebugPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetCompositePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetSkyPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTreeSpritePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetIcosahedronPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetWavesRenderPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetHighlightPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetBilateralBlurPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesDisturbPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesUpdatePSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetSobelPSODesc();

	D3D12_RENDER_TARGET_BLEND_DESC GetTransparentBlendState();
	D3D12_SHADER_BYTECODE GetShaderByteCode(const string& ShaderName);
	void BuildDescriptorHeap();
	void BuildPSOs();
	void BuildBlurPSO();
	void BuildShadersAndInputLayouts();

	ComPtr<IDXGIFactory2> GetFactory() const;

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;

	uint32_t GetDesiredCountOfRTVs() const;
	uint32_t GetDesiredCountOfDSVs() const;
	uint32_t GetDesiredCountOfSRVs() const;
	std::unordered_map<string, unique_ptr<SMeshGeometry>>& GetSceneGeometry();
	SMeshGeometry* SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry);

	vector<SInstanceData>& BuildRenderItemFromMesh(string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params);
	vector<SInstanceData>& BuildRenderItemFromMesh(const string& Category, SMeshGeometry* Mesh, const SRenderItemParams& Params);
	vector<SInstanceData>& BuildRenderItemFromMesh(const string& Category, const string& Name, const string& Path, const EParserType Parser, ETextureMapType GenTexels, const SRenderItemParams& Params);
	vector<SInstanceData>& BuildRenderItemFromMesh(string Category, const string& Name, const OGeometryGenerator::SMeshData& Data);
	void BuildPickRenderItem();
	SMeshGeometry* CreateMesh(const string& Name, const string& Path, const EParserType Parser, ETextureMapType GenTexels);
	unique_ptr<SMeshGeometry> CreateMesh(const string& Name, const OGeometryGenerator::SMeshData& Data) const;

	SMeshGeometry* FindSceneGeometry(const string& Name) const;

	void BuildShaders(const wstring& ShaderPath, const string& VSShaderName, const string& PSShaderName,
	                  const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildShader(const wstring& ShaderPath, const string& ShaderName, const string& ShaderQualifier, const string& ShaderTarget, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildVSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildPSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildGSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildCSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildShader(const wstring& ShaderPath, const string& ShaderName, EShaderLevel ShaderType, std::optional<string> ShaderEntry = {}, const D3D_SHADER_MACRO* Defines = nullptr);

	ComPtr<ID3DBlob> GetShader(const string& ShaderName);

	void CreatePSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc);
	void CreatePSO(const string& PSOName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& PSODesc);
	ComPtr<ID3D12PipelineState> GetPSO(const string& PSOName);

	void AddRenderItem(string Category, unique_ptr<ORenderItem> RenderItem);
	void AddRenderItem(const vector<string>& Categories, unique_ptr<ORenderItem> RenderItem);

	const vector<unique_ptr<ORenderItem>>& GetAllRenderItems();
	void SetPipelineState(string PSOName);
	void SetPipelineState(SPSODescriptionBase* PSOInfo);

	void SetFog(DirectX::XMFLOAT4 Color, float Start, float Range);
	SPassConstants& GetMainPassCB();
	ComPtr<ID3D12DescriptorHeap>& GetSRVHeap();

	ID3D12RootSignature* GetDefaultRootSignature() const;
	ID3D12RootSignature* GetWavesRootSignature() const;

	vector<D3D12_INPUT_ELEMENT_DESC>& GetDefaultInputLayout();

	OBlurFilter* GetBlurFilter();
	OBilateralBlurFilter* GetBilateralBlurFilter();
	OSobelFilter* GetSobelFilter();
	void BuildFilters();

	template<typename T, typename... Args>
	T* BuildRenderObject(Args&&... Params);

	TUUID AddRenderObject(IRenderObject* RenderObject);

	void BuildOffscreenRT();
	OOffscreenTexture* GetOffscreenRT() const;
	void DrawFullScreenQuad();

	template<typename T>
	TUUID AddFilter();

	template<typename T>
	T* GetObjectByUUID(TUUID UUID, bool Checked = false);

	void SetObjectDescriptor();
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescHandleForTexture(STexture* Texture) const;
	void SetLightSources(const vector<SLight>& Lights);
	void SetAmbientLight(const DirectX::XMFLOAT3& Color);

	SDescriptorResourceData GetRTVDescriptorData() const;
	SDescriptorResourceData GetDSVDescriptorData() const;
	SDescriptorResourceData GetSRVDescriptorData() const;
	uint32_t GetPassCountRequired() const;

	void RebuildGeometry(string Name);
	void TryUpdateGeometry();
	void UpdateGeometryRequest(string Name);

	float GetDeltaTime() const;
	float GetTime() const;
	TRenderLayer& GetRenderLayers();

	void PerformFrustrumCulling();
	uint32_t GetTotalNumberOfInstances() const;

	OMaterialManager* GetMaterialManager() const
	{
		return MaterialManager.get();
	}

	OTextureManager* GetTextureManager() const
	{
		return TextureManager.get();
	}

	OMeshGenerator* GetMeshGenerator() const;

	void Pick(int32_t SX, int32_t SY);
	ORenderItem* GetPickedItem() const;

	SOnFrameResourceChanged OnFrameResourceChanged;
	ODynamicCubeMapRenderTarget* GetCubeRenderTarget() const;
	ODynamicCubeMapRenderTarget* BuildCubeRenderTarget(DirectX::XMFLOAT3 Center);
	void DrawRenderItems(string PSOType, string RenderLayer);
	void DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer);

	void UpdateMaterialCB() const;
	void UpdateObjectCB() const;
	void SetDescriptorHeap();
	OShaderCompiler* GetShaderCompiler() const;
	OUIManager* GetUIManager() const;

protected:
	void DrawRenderItemsImpl(SPSODescriptionBase* Desc, const vector<ORenderItem*>& RenderItems);
	template<typename T, typename... Args>
	TUUID BuildRenderObjectImpl(Args&&... Params);

	shared_ptr<OTest> GetTestByHWND(HWND Handler);
	OWindow* GetWindowByHWND(HWND Handler);
	void Destroy();

	OWindow* Window;
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);

	void BuildPostProcessRootSignature();
	void BuildDefaultRootSignature();
	void BuildBlurRootSignature();
	void BuildWavesRootSignature();
	void BuildBilateralBlurRootSignature();
	void UpdateFrameResource();
	void InitRenderGraph();

private:
	void RemoveRenderObject(TUUID UUID);
	void BuildFrameResource(uint32_t Count = 1);

	uint32_t PassCount = 1;
	uint32_t CurrentPass = 0;
	uint32_t CurrentNumMaterials = 0;
	uint32_t CurrentNumInstances = 0;

	OEngine() = default;
	void UpdateMainPass(const STimer& Timer);
	SPassConstants MainPassCB;

	ComPtr<IDXGIAdapter4> Adapter;
	ComPtr<ID3D12Device2> Device;

	unique_ptr<OCommandQueue> DirectCommandQueue;
	unique_ptr<OCommandQueue> ComputeCommandQueue;
	unique_ptr<OCommandQueue> CopyCommandQueue;

	bool bIsTearingSupported = false;

	bool Msaa4xState = false;
	UINT Msaa4xQuality = 0;

	map<HWND, shared_ptr<OTest>> Tests;
	ComPtr<IDXGIFactory4> Factory;

	TRenderLayer RenderLayers;
	vector<unique_ptr<ORenderItem>> AllRenderItems;

	std::unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	std::unordered_map<string, ComPtr<ID3D12PipelineState>> PSOs;

	TSceneGeometryMap SceneGeometry;

	ComPtr<ID3D12RootSignature> PostProcessRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> WavesRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> DefaultRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BlurRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BilateralBlurRootSignature = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	// filters
	TUUID BlurFilterUUID;
	TUUID SobelFilterUUID;
	TUUID BilateralFilterUUID;

	ComPtr<ID3D12DescriptorHeap> SRVDescriptorHeap;
	SRenderObjectDescriptor ObjectDescriptors;
	uint32_t SRVDescNum = 0;

	bool HasInitializedTests = false;
	STimer TickTimer;
	OOffscreenTexture* OffscreenRT = nullptr;
	ODynamicCubeMapRenderTarget* CubeRenderTarget = nullptr;

	OUIManager* UIManager;
	map<TUUID, unique_ptr<IRenderObject>> RenderObjects;
	int32_t LightCount = 0;
	bool FrustrumCullingEnabled = false;

	unique_ptr<OMeshGenerator> MeshGenerator;
	unique_ptr<OTextureManager> TextureManager;
	unique_ptr<OMaterialManager> MaterialManager;

	inline static OEngine* Engine = nullptr;

	std::optional<string> GeometryToRebuild;

	ORenderItem* PickedItem = nullptr;

	unique_ptr<OShaderCompiler> ShaderCompiler;
	unique_ptr<OGraphicsPipelineManager> PipelineManager;
	unique_ptr<ORenderGraph> RenderGraph;
};

template<typename T, typename... Args>
TUUID OEngine::BuildRenderObjectImpl(Args&&... Params)
{
	auto uuid = GenerateUUID();
	auto object = new T(std::forward<Args>(Params)...);
	object->SetID(uuid);
	object->InitRenderObject();
	RenderObjects[uuid] = unique_ptr<IRenderObject>(object);
	return uuid;
}

template<typename T, typename... Args>
T* OEngine::BuildRenderObject(Args&&... Params)
{
	return GetObjectByUUID<T>(BuildRenderObjectImpl<T>(std::forward<Args>(Params)...));
}

template<typename T>
T* OEngine::GetObjectByUUID(TUUID UUID, bool Checked)
{
	if (!RenderObjects.contains(UUID))
	{
		if (Checked)
		{
			LOG(Engine, Error, "Render object not found!");
		}
		return nullptr;
	}
	const auto obj = RenderObjects[UUID].get();
	return Cast<T>(obj);
}

template<typename T>
TUUID OEngine::AddFilter()
{
	return AddRenderObject(OFilterBase::CreateFilter<T>(Device.Get(), GetCommandQueue(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat));
}