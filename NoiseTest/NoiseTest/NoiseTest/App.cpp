#include "App.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#include "../../../include/mlnoise/BlockNoise.h"
#include "../../../include/mlnoise/PerlinNoise.h"
#include "../../../include/mlnoise/BlockNoise.h"

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

        m_p3DContext->WaitForLastSubmittedFrame();

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
            m_p3DContext->WaitForLastSubmittedFrame();
            m_p3DContext->CleanupRenderTarget();
            m_p3DContext->ResizeBuffer(width, height);
            m_p3DContext->CreateRenderTarget();
        }

        return true;
    }

    void App::OnPaint()
    {
        //PAINTSTRUCT ps;
        //BeginPaint(m_hWnd, &ps);

        //m_pRTForD2D->BeginDraw();

        //m_pRTForD2D->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        //auto size = m_pBitmap->GetSize();
        //m_pRTForD2D->DrawBitmap(m_pBitmap, D2D1::RectF(0, 0, size.width, size.height));

        //auto hr = m_pRTForD2D->EndDraw();
        //EndPaint(m_hWnd, &ps);
    }

    bool App::Init()
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;

        if (InitD3D() == false)
            return false;

        //m_pImgui = std::make_unique<Imgui>(m_hWnd, g_pd3dDevice, NUM_BACK_BUFFERS, g_pd3dSrvDescHeap);

        if (InitD2D() == false)
            return false;

        UpdateNoise();

        m_isInit = true;

        return true;
    }

    void App::Update()
    {
        //m_pImgui->Update();
    }

    void App::Render()
    {
        //m_pImgui->Render();
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

       /* m_pRTForD2D = m_p2DContext->CreateRT(m_hWnd, m_p3DContext->GetBackBuffer());
        if (m_pRTForD2D == nullptr)
            return false;*/

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

    Imgui::Imgui(HWND hWnd, ID3D12Device* pDevice, std::uint32_t buffeFrames, ID3D12DescriptorHeap* pDescHeap)
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
        ImGui_ImplDX12_Init(pDevice, buffeFrames,
            DXGI_FORMAT_R8G8B8A8_UNORM, pDescHeap,
            pDescHeap->GetCPUDescriptorHandleForHeapStart(),
            pDescHeap->GetGPUDescriptorHandleForHeapStart());
    }

    Imgui::~Imgui()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Imgui::Update()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);

            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

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
        float dpi = GetDpiForWindow(hWnd);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

        HRESULT hr = m_pFactory->CreateDxgiSurfaceRenderTarget(pBuffer,
            props,
            &pRT);

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
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC1 sd;
        {
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = NUM_BACK_BUFFERS;
            sd.Width = 0;
            sd.Height = 0;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            sd.Scaling = DXGI_SCALING_STRETCH;
            sd.Stereo = FALSE;
        }

        // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
        ID3D12Debug* pdx12Debug = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
            pdx12Debug->EnableDebugLayer();
#endif

        // Create device
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
            return false;

        // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
        if (pdx12Debug != nullptr)
        {
            ID3D12InfoQueue* pInfoQueue = nullptr;
            g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            pInfoQueue->Release();
            pdx12Debug->Release();
        }
#endif

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                return false;

            SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                g_mainRenderTargetDescriptor[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                return false;
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
                return false;
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
                return false;

        if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
            g_pd3dCommandList->Close() != S_OK)
            return false;

        if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
            return false;

        g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (g_fenceEvent == nullptr)
            return false;

        {
            IDXGIFactory4* dxgiFactory = nullptr;
            IDXGISwapChain1* swapChain1 = nullptr;
            if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
                return false;
            if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
                return false;
            if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
                return false;
            swapChain1->Release();
            dxgiFactory->Release();
            g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
            g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        }

        CreateRenderTarget();
        return true;
    }

    void D3DContext::WaitForLastSubmittedFrame()
    {
        FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

        UINT64 fenceValue = frameCtx->FenceValue;
        if (fenceValue == 0)
            return; // No fence was signaled

        frameCtx->FenceValue = 0;
        if (g_fence->GetCompletedValue() >= fenceValue)
            return;

        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    bool D3DContext::ResizeBuffer(std::uint32_t w, std::uint32_t h)
    {
        HRESULT res = g_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        return SUCCEEDED(res);
    }

    void D3DContext::WaitFence()
    {
        D3DContext::FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        //ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    IDXGISurface* D3DContext::GetBackBuffer()
    {
        IDXGISurface* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        return pBackBuffer;
    }

    D3DContext::FrameContext* D3DContext::WaitForNextFrameResources()
    {
        UINT nextFrameIndex = g_frameIndex + 1;
        g_frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
        UINT64 fenceValue = frameCtx->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameCtx->FenceValue = 0;
            g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
            waitableObjects[1] = g_fenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameCtx;
    }

    void D3DContext::CreateRenderTarget()
    {
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            ID3D12Resource* pBackBuffer = nullptr;
            g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }

    void D3DContext::CleanupRenderTarget()
    {
        WaitForLastSubmittedFrame();

        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
    }

    void D3DContext::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
        if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
        if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
        if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
        if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
        if (g_fence) { g_fence->Release(); g_fence = nullptr; }
        if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
        IDXGIDebug1* pDebug = nullptr;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            pDebug->Release();
        }
#endif
    }
}