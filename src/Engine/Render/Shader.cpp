#include "Shader.h"
#include "Common/Logger.h"
#include "d3dcompiler.h"

namespace IHA::Engine {

    bool Shader::Load(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device)
    {
        HRESULT hr;

        // compile .hlsl
        hr = D3DCompileFromFile(vsPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VS", "vs_5_0", 0, 0, &vertexBlob, nullptr);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to compile vertex shader");
            return false;
        }

        hr = D3DCompileFromFile(psPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PS", "ps_5_0", 0, 0, &pixelBlob, nullptr);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to compile pixel shader");
            return false;
        }

        D3D12_DESCRIPTOR_RANGE texRange = {};
        texRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        texRange.NumDescriptors = 1;
        texRange.BaseShaderRegister = 0; // t0
        texRange.RegisterSpace = 0;
        texRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // Root parameters
        D3D12_ROOT_PARAMETER rootParams[2] = {};

        // root param 0 : CBV b0
        rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParams[0].Descriptor.ShaderRegister = 0;    // b0
        rootParams[0].Descriptor.RegisterSpace = 0;
        rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // root param 1 : texture SRV t0
        rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
        rootParams[1].DescriptorTable.pDescriptorRanges = &texRange;
        rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // Root signature desc
        D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
        rootSigDesc.NumParameters = _countof(rootParams);
        rootSigDesc.pParameters = rootParams;
        rootSigDesc.NumStaticSamplers = 0;
        rootSigDesc.pStaticSamplers = nullptr;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        // Serialize & create
        ComPtr<ID3DBlob> serializedRootSig;
        ComPtr<ID3DBlob> errorBlob;
        hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                LOG_ERROR((char*)errorBlob->GetBufferPointer());
            }
            else {
                LOG_ERROR("D3D12SerializeRootSignature failed.");
            }
            return false;
        }

        hr = device->CreateRootSignature(0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create root signature");
            return false;
        }

        // 3) Input layout
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS; // D3D12_DEFAULT_DEPTH_BIAS == 0
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.SlopeScaledDepthBias = 0.0f;
        rasterDesc.DepthClipEnable = TRUE;
        rasterDesc.MultisampleEnable = FALSE;
        rasterDesc.AntialiasedLineEnable = FALSE;
        rasterDesc.ForcedSampleCount = 0;
        rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        for (int i = 0; i < 8; ++i) {
            blendDesc.RenderTarget[i].BlendEnable = FALSE;
            blendDesc.RenderTarget[i].LogicOpEnable = FALSE;
            blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
            blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = TRUE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        depthDesc.StencilEnable = FALSE;
        depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthDesc.BackFace = depthDesc.FrontFace;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        psoDesc.pRootSignature = rootSignature.Get();

        psoDesc.VS.pShaderBytecode = vertexBlob->GetBufferPointer();
        psoDesc.VS.BytecodeLength = vertexBlob->GetBufferSize();

        psoDesc.PS.pShaderBytecode = pixelBlob->GetBufferPointer();
        psoDesc.PS.BytecodeLength = pixelBlob->GetBufferSize();

        psoDesc.RasterizerState = rasterDesc;
        psoDesc.BlendState = blendDesc;
        psoDesc.DepthStencilState = depthDesc;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create PSO");
            return false;
        }

        return true;
    }

    void Shader::Bind(ID3D12GraphicsCommandList* cmd)
    {
        cmd->SetPipelineState(pso.Get());
        cmd->SetGraphicsRootSignature(rootSignature.Get());
    }

}