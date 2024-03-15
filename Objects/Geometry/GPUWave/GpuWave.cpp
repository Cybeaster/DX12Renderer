#include "GpuWave.h"

OGPUWave::OGPUWave(ID3D12Device* _Device, ID3D12GraphicsCommandList* _List, int32_t _M, int32_t _N, float dx, float dt, float speed, float damping)
    : Device(_Device), CMDList(_List), NumRows(_M), NumCols(_N), SpatialStep(dx), TimeStep(dt), VertexCount(_M * _N), TriangleCount((_M - 1) * (_N - 1) * 2)
{
	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);

	mK[0] = (damping * dt - 2.0f) / d;
	mK[1] = (4.0f - 8.0f * e) / d;
	mK[2] = (2.0f * e) / d;
	BuildResources();
}

void OGPUWave::BuildResources()
{
	// All the textures for the wave simulation will be bound as a shader resource and
	// unordered access view at some point since we ping-pong the buffers.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = NumCols;
	texDesc.Height = NumRows;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	NextSol = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
	PrevSol = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
	CurrSol = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);

	//
	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap.
	//

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(CurrSol.Resource.Get(), 0, num2DSubresources);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	PrevUploadBuffer = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_UPLOAD, resourceDesc);
	CurrUploadBuffer = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_UPLOAD, resourceDesc);

	//describe the data, we're going to copy into the default buffer
	vector<float> initData(NumRows * NumCols, 0.0f);

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.data();
	subResourceData.RowPitch = NumCols * sizeof(float);
	subResourceData.SlicePitch = subResourceData.RowPitch * NumRows;

	//
	// Schedule to copy the data to the default resource, and change states.
	// Note that mCurrSol is put in the GENERIC_READ state so it can be
	// read by a shader.
	//

	Utils::ResourceBarrier(CMDList, &PrevSol, D3D12_RESOURCE_STATE_COPY_DEST);

	UpdateSubresources(CMDList, PrevSol.Resource.Get(), PrevUploadBuffer.Resource.Get(), 0, 0, num2DSubresources, &subResourceData);

	Utils::ResourceBarrier(CMDList, &PrevSol, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Utils::ResourceBarrier(CMDList, &CurrSol, D3D12_RESOURCE_STATE_COPY_DEST);

	UpdateSubresources(CMDList, CurrSol.Resource.Get(), CurrUploadBuffer.Resource.Get(), 0, 0, num2DSubresources, &subResourceData);

	Utils::ResourceBarrier(CMDList, &CurrSol, D3D12_RESOURCE_STATE_GENERIC_READ);
	Utils::ResourceBarrier(CMDList, &NextSol, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void OGPUWave::BuildDescriptors(IDescriptor* Descriptor)
{
	auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	descriptor->SRVHandle.Offset(PrevSolSRVHandle);
	descriptor->SRVHandle.Offset(CurrSolSRVHandle);
	descriptor->SRVHandle.Offset(NextSolSRVHandle);

	descriptor->SRVHandle.Offset(PrevSolUAVHandle);
	descriptor->SRVHandle.Offset(CurrSolUAVHandle);
	descriptor->SRVHandle.Offset(NextSolUAVHandle);

	Device->CreateShaderResourceView(PrevSol.Resource.Get(), &srvDesc, PrevSolSRVHandle.CPUHandle);
	Device->CreateShaderResourceView(CurrSol.Resource.Get(), &srvDesc, CurrSolSRVHandle.CPUHandle);
	Device->CreateShaderResourceView(NextSol.Resource.Get(), &srvDesc, NextSolSRVHandle.CPUHandle);

	Device->CreateUnorderedAccessView(PrevSol.Resource.Get(), nullptr, &uavDesc, PrevSolUAVHandle.CPUHandle);
	Device->CreateUnorderedAccessView(CurrSol.Resource.Get(), nullptr, &uavDesc, CurrSolUAVHandle.CPUHandle);
	Device->CreateUnorderedAccessView(NextSol.Resource.Get(), nullptr, &uavDesc, NextSolUAVHandle.CPUHandle);
}

void OGPUWave::Update(const STimer& Gt, ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO)
{
	static float t = 0.0f;

	t += Gt.GetDeltaTime();

	CMDList->SetPipelineState(PSO);
	CMDList->SetComputeRootSignature(RootSignature);

	if (t >= TimeStep)
	{
		CMDList->SetComputeRoot32BitConstants(0, 3, mK, 0);

		CMDList->SetComputeRootDescriptorTable(1, PrevSolSRVHandle.GPUHandle);
		CMDList->SetComputeRootDescriptorTable(2, CurrSolUAVHandle.GPUHandle);
		CMDList->SetComputeRootDescriptorTable(3, NextSolUAVHandle.GPUHandle);

		// How many groups do we need to dispatch to cover the wave grid.
		// Note that mNumRows and mNumCols should be divisible by 16
		// so there is no remainder.

		const UINT numGroupsX = NumCols / 16;
		const UINT numGroupsY = NumRows / 16;
		CMDList->Dispatch(numGroupsX, numGroupsY, 1);

		//
		// Ping-pong buffers in preparation for the next update.
		// The previous solution is no longer needed and becomes the target of the next solution in the next update.
		// The current solution becomes the previous solution.
		// The next solution becomes the current solution.
		//

		auto resTmp = PrevSol;
		PrevSol = CurrSol;
		CurrSol = NextSol;
		NextSol = resTmp;

		auto srvTmp = PrevSolSRVHandle;
		PrevSolSRVHandle = CurrSolSRVHandle;
		CurrSolSRVHandle = NextSolSRVHandle;
		NextSolSRVHandle = srvTmp;

		auto uavTemp = PrevSolUAVHandle;
		PrevSolUAVHandle = CurrSolUAVHandle;
		CurrSolUAVHandle = NextSolUAVHandle;
		NextSolUAVHandle = uavTemp;

		t = 0.0;

		// The current solution needs to be able to be read by the vertex shader, so change its state to GENERIC_READ.
		Utils::ResourceBarrier(CMDList, &CurrSol, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}

void OGPUWave::Disturb(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, UINT I, UINT J, float Magnitude)
{
	CMDList->SetPipelineState(PSO);
	CMDList->SetComputeRootSignature(RootSignature);

	const UINT disturbIdx[2] = { J, I };
	CMDList->SetComputeRoot32BitConstants(0, 1, &Magnitude, 3);
	CMDList->SetComputeRoot32BitConstants(0, 2, disturbIdx, 4);

	CMDList->SetComputeRootDescriptorTable(3, CurrSolUAVHandle.GPUHandle);

	// The current solution is in the GENERIC_READ state so it can be read by the vertex shader.
	// Change it to UNORDERED_ACCESS for the compute shader.  Note that a UAV can still be
	// read in a compute shader.

	Utils::ResourceBarrier(CMDList, &CurrSol, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	CMDList->Dispatch(1, 1, 1);
}
void OGPUWave::Update(const UpdateEventArgs& Event)
{
	auto engine = OEngine::Get();
	auto commandList = engine->GetCommandQueue()->GetCommandList();
	const auto wavesRootSignature = engine->GetWavesRootSignature();
	static float tBase = 0.0f;
	if (Event.Timer.GetTime() - tBase >= 0.25)
	{
		tBase += 0.25;
		int i = Utils::Math::Random(4, GetRowCount() - 5);
		int j = Utils::Math::Random(4, GetColumnCount() - 5);
		float r = Utils::Math::Random(1.f, 2.f);

		//Disturb(wavesRootSignature, engine->GetPSO(SPSOType::WavesDisturb).Get(), i, j, r);
	}
	//Update(Event.Timer, wavesRootSignature, engine->GetPSO(SPSOType::WavesUpdate).Get());
}