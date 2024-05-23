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
        SafeRelease(&m_pRTForD2D);
        SafeRelease(&m_pBitmap);

        //m_p3DContext->WaitForLastSubmittedFrame();

        m_pImgui.reset();
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
        if (m_pRTForD2D != nullptr)
        {
            RECT rc;
            GetClientRect(m_hWnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

            //m_pRTForD2D->Resize(size);
            InvalidateRect(m_hWnd, nullptr, false);

            m_width = width;
            m_height = height;
            UpdateNoise();
        }

        if (m_p3DContext != nullptr)
        {
            //m_p3DContext->WaitForLastSubmittedFrame();
            m_p3DContext->CleanupRenderTarget();
            m_p3DContext->ResizeBuffer(width, height);
            m_p3DContext->CreateRenderTarget();
        }

        return true;
    }

    void App::OnPaint()
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hWnd, &ps);

        m_pRTForD2D->BeginDraw();

        m_pRTForD2D->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        auto size = m_pBitmap->GetSize();
        m_pRTForD2D->DrawBitmap(m_pBitmap, D2D1::RectF(0, 0, size.width, size.height));

        auto hr = m_pRTForD2D->EndDraw();
        EndPaint(m_hWnd, &ps);
    }

    bool App::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
    }

    bool App::Init()
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;

        if (InitD3D() == false)
            return false;

        if (InitD2D() == false)
            return false;

        m_pImgui = std::make_unique<Imgui>(m_hWnd, m_p3DContext->GetDevice(), 1, m_p3DContext->GetDeviceContext());

        UpdateNoise();

        m_isInit = true;

        return true;
    }

    void App::Update()
    {
        m_pImgui->Update();
    }

    void App::Render()
    {
        m_pImgui->Render();
        std::array<ID3D11RenderTargetView*, 1> ppRTVs = { { m_p3DContext->pRTV() } };
        m_p3DContext->GetDeviceContext()->OMSetRenderTargets(1, ppRTVs.data(), nullptr);
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        //m_p3DContext->GetDeviceContext()->ClearRenderTargetView(m_p3DContext->pRTV(), clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void App::SwapFrame()
    {
        m_p3DContext->WaitFence();
    }

    bool App::InitD2D()
    {
        m_p2DContext = std::make_unique<D2DContext>();
        if (m_p2DContext->Init(m_hWnd) == false)
            return false;

        m_pRTForD2D = m_p2DContext->CreateRT(m_hWnd, m_p3DContext->GetBackBuffer());
        if (m_pRTForD2D == nullptr)
            return false;

        return true;
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
        if (m_pRTForD2D == nullptr)
            return;

        mlnoise::PerlinNoise<std::float_t> noise;

        constexpr auto octarve = 1;
        constexpr auto freq = 1.0f / 128;
        constexpr auto amp = 0.5f;

        auto const w = m_width;
        auto const h = m_width;

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

        m_pBitmap = m_p2DContext->CreateBMP(m_pRTForD2D, m_width, m_width, pData);

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

    void Imgui::Update()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(300, 250));
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
    }

    void Imgui::Render()
    {
        ImGui::Render();
    }
}

namespace app
{
    D2DContext::~D2DContext()
    {
        SafeRelease(&m_pFactory);
    }

    bool D2DContext::Init(HWND hWnd)
    {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
        if (FAILED(hr))
            return false;

        return true;
    }

    ID2D1RenderTarget* D2DContext::CreateRT(HWND hWnd, IDXGISurface* pBuffer)
    {
        ID2D1RenderTarget* pRT = nullptr;

        RECT rc;
        GetClientRect(hWnd, &rc);

        auto size = D2D1::SizeU(rc.right, rc.bottom);
        /*HRESULT hr = m_pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                      D2D1::HwndRenderTargetProperties(hWnd, size),
                                                      &pRT);*/
        FLOAT dpiX = 0.0f;
        FLOAT dpiY = 0.0f;
        //m_pFactory->GetDesktopDpi(&dpiX, &dpiY);
        float dpi = GetDpiForWindow(hWnd);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi, dpi);

        HRESULT hr = m_pFactory->CreateDxgiSurfaceRenderTarget(pBuffer, &props, &pRT);

        if (FAILED(hr))
            return nullptr;

        return pRT;
    }

    ID2D1Bitmap* D2DContext::CreateBMP(ID2D1RenderTarget* pRT, std::uint32_t w, std::uint32_t h, BYTE* pBuff)
    {
        ID2D1Bitmap* pBMP = nullptr;
        auto prop = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        auto hr = pRT->CreateBitmap(D2D1::SizeU(w, h), pBuff, w * 4, prop, &pBMP);
        if (FAILED(hr))
            return nullptr;

        return pBMP;
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
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    bool D3DContext::ResizeBuffer(std::uint32_t w, std::uint32_t h)
    {
        HRESULT res = g_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        return SUCCEEDED(res);
    }

    void D3DContext::WaitFence()
    {
        g_pSwapChain->Present(0, 0);

        //D3DContext::FrameContext* frameCtx = WaitForNextFrameResources();
        //UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        //frameCtx->CommandAllocator->Reset();

        //D3D12_RESOURCE_BARRIER barrier = {};
        //barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        //barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        //barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
        //barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        //barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        //barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        //g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        //g_pd3dCommandList->ResourceBarrier(1, &barrier);

        //// Render Dear ImGui graphics
        //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        //const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        //g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        //g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        //g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ////ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        //barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        //barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        //g_pd3dCommandList->ResourceBarrier(1, &barrier);
        //g_pd3dCommandList->Close();

        //g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        //g_pSwapChain->Present(1, 0); // Present with vsync
        ////g_pSwapChain->Present(0, 0); // Present without vsync

        //UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        //g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        //g_fenceLastSignaledValue = fenceValue;
        //frameCtx->FenceValue = fenceValue;
    }

    IDXGISurface* D3DContext::GetBackBuffer()
    {
        IDXGISurface* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        return pBackBuffer;
    }

    ID3D11Device* D3DContext::GetDevice()
    {
        return g_pd3dDevice;
    }

    ID3D11DeviceContext* D3DContext::GetDeviceContext()
    {
        return g_pd3dDeviceContext;
    }

    ID3D11RenderTargetView* D3DContext::pRTV()
    {
        return g_mainRenderTargetView;
    }

    void D3DContext::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void D3DContext::CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    }

    void D3DContext::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    }
}