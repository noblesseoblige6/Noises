#include "App.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "../../../include/mlnoise/BlockNoise.h"
#include "../../../include/mlnoise/PerlinNoise.h"
#include "../../../include/mlnoise/BlockNoise.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
        SafeRelease(&m_pTex);

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
        m_width = width;
        m_height = height;

        m_isResized = true;

        return true;
    }

    void App::OnPaint()
    {
    }

    bool App::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
        return false;
    }

    bool App::Init()
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;

        if (InitD3D() == false)
            return false;

        m_pImgui = std::make_unique<Imgui>(m_hWnd, m_p3DContext->GetDevice(), 1, m_p3DContext->GetDeviceContext());

        UpdateNoise();

        m_isInit = true;

        return true;
    }

    void App::Update()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(m_width, m_height));
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.DisplaySize = ImVec2(m_width, m_height);

        m_pImgui->Begin();
        {
            ImGui::Begin("Noise properties");

            ImGui::InputInt("Size", &m_size, 0, 16);

            ImGui::Combo("Noise", &m_noiseIndex, "Block\0Value\0Perlin\0\0");

            ImGui::SliderInt("Octave", &m_octave, 1, 16);
            ImGui::SliderFloat("Frequency", &m_frequency, 0.01f, 64.f);
            ImGui::SliderFloat("Persistence", &m_persistence, 0.01f, 0.5f);

            if (ImGui::Button("Generate"))
            {
                UpdateNoise();
                //InvalidateRect(m_hWnd, nullptr, false);
            }

            ImGui::End();

            ImGui::Begin("Preview");
            ImGui::Image((void*)m_pTex, ImVec2(m_size, m_size));

            ImGui::End();
        }
        m_pImgui->End();

        if (m_isResized && !m_beginResize)
        {
            m_beginResize = true;
            m_isResized = false;
        }
        else if(!m_isResized && m_beginResize)
        {
            std::cout << "Resize" << std::endl;

            m_beginResize = false;
            m_isResized = false;

            if (m_p3DContext != nullptr)
            {
                m_p3DContext->CleanupRenderTarget();
                m_p3DContext->ResizeBuffer(m_width, m_height);
                m_p3DContext->CreateRenderTarget();
            }
        }
        else
        {
            m_isResized = false;
        }
    }

    void App::Render()
    {
        m_pImgui->Render();
        std::array<ID3D11RenderTargetView*, 1> ppRTVs = { { m_p3DContext->pRTV() } };
        m_p3DContext->GetDeviceContext()->OMSetRenderTargets(1, ppRTVs.data(), nullptr);
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
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

    void App::UpdateNoise()
    {
        mlnoise::PerlinNoise<std::float_t> noise;

        auto const octarve = m_octave;
        auto const freq = m_frequency;
        auto const amp = m_persistence;

        //FIXME: why is the ratio 1 to 1?
        auto const w = m_size;
        auto const h = m_size;

        UINT8* pData = new UINT8[w * h * 4];
        for (auto j = 0; j < h; j++)
        {
            for (auto i = 0; i < w; i++)
            {
                auto res = noise.Fractal_01(i * freq, j * freq, octarve, amp);

                unsigned char v = 255 * res;

                UINT8* pPixelData = pData + (i + (j * h)) * 4;
                pPixelData[0] = v;
                pPixelData[1] = v;
                pPixelData[2] = v;
                pPixelData[3] = 255;
            }
        }

        SafeRelease(&m_pTex);
        m_pTex = m_p3DContext->CreateTexture(w, h, pData);

        delete[] pData;
    }

    Imgui::Imgui(HWND hWnd, ID3D11Device* pDevice, std::uint32_t buffeFrames, ID3D11DeviceContext* pContext)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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

    void Imgui::End()
    {
        //ImGui::End();
    }

    void Imgui::Render()
    {
        ImGui::Render();
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
        sd.BufferCount = 1;
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
        //sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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
        m_pSwapChain->Present(0, 0);
    }

    ID3D11ShaderResourceView* D3DContext::CreateTexture(std::uint32_t w, std::uint32_t h, UINT8* pData)
    {
        // Create texture
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

        ID3D11ShaderResourceView* pOutTex = nullptr;
        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pOutTex);
        pTexture->Release();

        return pOutTex;
    }

    IDXGISurface* D3DContext::GetBackBuffer()
    {
        IDXGISurface* pBackBuffer = nullptr;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        return pBackBuffer;
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