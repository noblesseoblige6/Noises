#include "App.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "../../../include/mlnoise/BlockNoise.h"
#include "../../../include/mlnoise/ValueNoise.h"
#include "../../../include/mlnoise/PerlinNoise.h"
#include "../../../include/mlnoise/SimplexNoise.h"
#include "../../../include/mlnoise/VoronoiNoise.h"


namespace app
{
    template<class Interface>
    inline void SafeRelease(Interface** ppInterfaceToRelease)
    {
        if (*ppInterfaceToRelease != NULL)
        {
            (*ppInterfaceToRelease)->Release();
            (*ppInterfaceToRelease) = NULL;
        }
    }

    App::App(HWND hWnd, HINSTANCE hInst)
        : m_hWnd(hWnd)
        , m_hInst(hInst)
    {
    }

    App::~App()
    {
        SafeRelease(&m_pNoiseTex);

        m_pImgui.reset();
    }

    bool App::Run()
    {
        if (!m_isInit)
            return Init();

        Update();
        Render();

        SwapFrame();

        return true;
    }

    bool App::OnResize(std::uint32_t width, std::uint32_t height)
    {
        m_isResized = true;

        m_clientSize =
        {
            std::get<0>(m_clientSize),
            std::get<1>(m_clientSize),
            width,
            height
        };

        return true;
    }

    bool App::Init()
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        m_clientSize = { rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top };
        UpdateWindowSize();


        if (InitD3D() == false)
            return false;

        m_pImgui = std::make_unique<Imgui>(m_hWnd, m_p3DContext->GetDevice(), 1, m_p3DContext->GetDeviceContext());

        UpdateNoise();
        UpdateNoiseTex();

        m_isInit = true;

        return true;
    }

    void App::Update()
    {
        bool canIgnoreChange = false;

        if (m_future.valid())
        {
            auto status = m_future.wait_for(std::chrono::seconds(1));
            if (status != std::future_status::ready)
            {
                canIgnoreChange = true;
            }
            else
            {
                UpdateNoiseTex();
                m_future = std::future<void>();
            }
        }

        bool isChanged = false;

        m_pImgui->Begin();
        {
            ImGui::SetNextWindowPos (ImVec2(std::get<0>(m_propertySize), std::get<1>(m_propertySize)));
            ImGui::SetNextWindowSize(ImVec2(std::get<2>(m_propertySize), std::get<3>(m_propertySize)));

            ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);

            isChanged |= ImGui::Combo("Noise", &m_noiseType, "Block\0Value\0Perlin\0Simplex\0Voronoi\0\0");
            isChanged |= ImGui::SliderFloat("Frequency", &m_frequency, 0.001f, 1.f);
            isChanged |= ImGui::Combo("SmoothStep", &m_smopthStepType, "Quintic\0Cubic\0Linear\0\0");
            ImGui::Text("Fractal");
            isChanged |= ImGui::SliderInt("Octave", &m_octave, 1, 8);
            isChanged |= ImGui::SliderFloat("Persistence", &m_persistence, 0.0f, 1.0f);
            isChanged |= ImGui::SliderFloat("Lacunarity", &m_lacunarity, 0.0f, 5.0f);

            
            ImGui::End();

            ImGui::SetNextWindowPos (ImVec2(std::get<0>(m_previewSize), std::get<1>(m_previewSize)));
            ImGui::SetNextWindowSize(ImVec2(std::get<2>(m_previewSize), std::get<3>(m_previewSize)));

            ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

            ImGui::Image((void*)m_pNoiseTex, ImVec2(std::get<0>(m_texSize), std::get<1>(m_texSize)));

            ImGui::End();

            //bool showDemo = true;
            //ImGui::ShowDemoWindow(&showDemo);
        }

        if (isChanged == false)
            return;

        if (canIgnoreChange)
        {
            m_keptChange = true;
            return;
        }

        UpdateSize();
        UpdateNoiseAsync();
        m_keptChange = false;
    }

    void App::Render()
    {
        ImGui::Render();

        std::array<ID3D11RenderTargetView*, 1> ppRTVs = { { m_p3DContext->pRTV() } };
        m_p3DContext->GetDeviceContext()->OMSetRenderTargets(1, ppRTVs.data(), nullptr);

        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        m_p3DContext->GetDeviceContext()->ClearRenderTargetView(m_p3DContext->pRTV(), clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void App::SwapFrame()
    {
        m_p3DContext->WaitFence();
    }

    bool App::InitD3D()
    {
        m_p3DContext = std::make_unique<D3DContext>();
        if (m_p3DContext->Init(m_hWnd) == false)
            return false;

        return true;
    }

    template<class T>
    inline void Noise(UINT8* pData,
                      std::int32_t w,
                      std::int32_t h,
                      std::int32_t seed, 
                      std::float_t freq, 
                      std::int32_t octarve, 
                      std::float_t amp, 
                      std::float_t lacunarity,
                      std::int32_t smoothStep)
    {
        T noise(seed);

        switch (static_cast<SmoothStepType>(smoothStep))
        {
        case SmoothStepType::Quintic:
        {
            noise.SetSmoothStep(mlnoise::detail::Fade_quintic<std::float_t>);
        }
        break;
        case SmoothStepType::Cubic:
        {
            noise.SetSmoothStep(mlnoise::detail::Fade_cubic<std::float_t>);
        }
        break;
        case SmoothStepType::Linear:
        {
            noise.SetSmoothStep(mlnoise::detail::Fade_linear<std::float_t>);
        }
        break;
        default:
            break;
        }

        noise.SetLacunarity(lacunarity);

        //#pragma omp parallel for
        for (auto j = 0; j < h; j++)
        {
            //#pragma omp parallel for
            for (auto i = 0; i < w; i++)
            {
                auto res = noise.Fractal_01(i * freq, j * freq, octarve, amp);

                unsigned char v = 255 * res;

                UINT8* pPixelData = pData + (i + (j * w)) * 4;
                pPixelData[0] = v;
                pPixelData[1] = v;
                pPixelData[2] = v;
                pPixelData[3] = 255;
            }
        }
    }

    void App::UpdateNoise()
    {
        auto const w = std::get<0>(m_texSize);
        auto const h = std::get<1>(m_texSize);

        m_pTexBuffer = new UINT8[w * h * 4];

        switch (static_cast<NoiseType>(m_noiseType))
        {
        case NoiseType::Block:
        {
            Noise<mlnoise::BlockNoise<std::float_t>>(m_pTexBuffer, w, h, m_seed, m_frequency, m_octave, m_persistence, m_lacunarity, m_smopthStepType);
        }
        break;
        case NoiseType::Value:
        {
            Noise<mlnoise::ValueNoise<std::float_t>>(m_pTexBuffer, w, h, m_seed, m_frequency, m_octave, m_persistence, m_lacunarity, m_smopthStepType);
        }
        break;
        case NoiseType::Perlin:
        {
            Noise<mlnoise::PerlinNoise<std::float_t>>(m_pTexBuffer, w, h, m_seed, m_frequency, m_octave, m_persistence, m_lacunarity, m_smopthStepType);
        }
        break;
        case NoiseType::Simplex:
        {
            //Noise<mlnoise::SimplexNoise<std::float_t>>(m_pTexBuffer, w, h, m_seed, m_frequency, m_octave, m_persistence, m_lacunarity, m_smopthStepType);
            mlnoise::SimplexNoise<std::float_t> noise;
            for (auto j = 0u; j < h; j++)
            {
                for (auto i = 0u; i < w; i++)
                {
                    UINT8* pPixelData = m_pTexBuffer + (i + (j * static_cast<std::int32_t>(w))) * 4;
                    pPixelData[0] = noise.NoiseX(i * m_frequency, j * m_frequency) * 255;
                    pPixelData[1] = noise.NoiseY(i * m_frequency, j * m_frequency) * 255;
                    pPixelData[2] = noise.NoiseZ(i * m_frequency, j * m_frequency) * 255;
                    pPixelData[3] = 255;
                }
            }
        }
        break;
        case NoiseType::Voronoi:
        {
            Noise<mlnoise::VoronoiNoise<std::float_t>>(m_pTexBuffer, w, h, m_seed, m_frequency, m_octave, m_persistence, m_lacunarity, m_smopthStepType);
        }
        break;
        default:
            break;
        }
    }

    void App::UpdateNoiseAsync()
    {
        m_future = std::async(std::launch::async, [&] { UpdateNoise(); });
    }

    void App::UpdateSize()
    {
        if (m_isResized == false)
            return;

        m_isResized = false;

        UpdateWindowSize();

        if (m_p3DContext != nullptr)
        {
            m_p3DContext->CleanupRenderTarget();
            m_p3DContext->ResizeBuffer(std::get<2>(m_clientSize), std::get<3>(m_clientSize));
            m_p3DContext->CreateRenderTarget();
        }
    }

    void App::UpdateWindowSize()
    {
        m_propertySize =
        {
            std::get<0>(m_clientSize),
            std::get<1>(m_clientSize),
            std::get<2>(m_clientSize) / 4,
            std::get<3>(m_clientSize),
        };

        m_previewSize =
        {
            std::get<0>(m_clientSize) + std::get<2>(m_propertySize),
            std::get<1>(m_clientSize),
            std::get<2>(m_clientSize) - std::get<2>(m_propertySize),
            std::get<3>(m_clientSize),
        };

        m_texSize = { std::get<2>(m_previewSize), std::get<3>(m_previewSize) /*- 35*/ };
    }

    void App::UpdateNoiseTex()
    {
        auto const w = std::get<0>(m_texSize);
        auto const h = std::get<1>(m_texSize);

        SafeRelease(&m_pNoiseTex);
        m_pNoiseTex = m_p3DContext->CreateTexture(w, h, m_pTexBuffer);

        delete[] m_pTexBuffer;
        m_pTexBuffer = nullptr;
    }

    Imgui::Imgui(HWND hWnd, ID3D11Device* pDevice, std::uint32_t buffeFrames, ID3D11DeviceContext* pContext)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX11_Init(pDevice, pContext);
    }

    Imgui::~Imgui()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Imgui::Begin()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }
}

namespace app
{
    D3DContext::~D3DContext()
    {
        CleanupDeviceD3D();
    }

    bool D3DContext::Init(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    bool D3DContext::ResizeBuffer(std::uint32_t w, std::uint32_t h)
    {
        HRESULT res = m_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        return SUCCEEDED(res);
    }

    void D3DContext::WaitFence()
    {
        m_pSwapChain->Present(1, 0);
    }

    ID3D11ShaderResourceView* D3DContext::CreateTexture(std::uint32_t w, std::uint32_t h, UINT8* pData)
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = pData;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        m_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        ID3D11ShaderResourceView* pOutTex = nullptr;
        m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pOutTex);
        pTexture->Release();

        return pOutTex;
    }

    ID3D11Device* D3DContext::GetDevice()
    {
        return m_pd3dDevice;
    }

    ID3D11DeviceContext* D3DContext::GetDeviceContext()
    {
        return m_pd3dDeviceContext;
    }

    ID3D11RenderTargetView* D3DContext::pRTV()
    {
        return m_pRTV;
    }

    void D3DContext::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer{nullptr};
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRTV);
        pBackBuffer->Release();
    }

    void D3DContext::CleanupRenderTarget()
    {
        SafeRelease(&m_pRTV);
    }

    void D3DContext::CleanupDeviceD3D()
    {
        CleanupRenderTarget();

        SafeRelease(&m_pSwapChain);
        SafeRelease(&m_pd3dDeviceContext);
        SafeRelease(&m_pd3dDevice);
    }
}