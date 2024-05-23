#pragma once
#include <memory>

#include <wtypes.h>
#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")

#define DX12_ENABLE_DEBUG_LAYER

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace app
{
    class D2DContext;
    class D3DContext;
    class Imgui;
    class App
    {
    public:
        App(HWND hWnd, HINSTANCE hInst);
        ~App();

    public:
        bool Run();
        bool OnResize(std::uint32_t width, std::uint32_t height);
        void OnPaint();

    private:
        bool Init();
        void Update();
        void Render();
        void SwapFrame();

        bool InitD2D();
        bool InitD3D();
        void UpdateNoise();

    private:
        HWND          m_hWnd;
        HINSTANCE     m_hInst;

        bool m_isInit{ false };

        std::uint32_t m_width{1024};
        std::uint32_t m_height{912};

        std::unique_ptr<Imgui> m_pImgui{nullptr};

        std::unique_ptr <D3DContext> m_p3DContext{ nullptr };
        std::unique_ptr <D2DContext> m_p2DContext{ nullptr };

        ID2D1RenderTarget* m_pRTForD2D{ nullptr };
        ID2D1Bitmap* m_pBitmap{ nullptr };
    };

    class Imgui
    {
    public:
        Imgui(HWND hWnd, ID3D12Device* pDevice, std::uint32_t buffeFrames, ID3D12DescriptorHeap* pDescHeap);
        ~Imgui();

    public:
        void Update();
        void Render();
    };
}

namespace app
{
    class D2DContext
    {
    public:
        D2DContext() = default;
        ~D2DContext();

    public:
        bool Init(HWND hWnd);
        ID2D1RenderTarget* CreateRT(HWND hWnd, IDXGISurface* pBuffer);
        ID2D1Bitmap* CreateBMP(ID2D1RenderTarget* pRT, std::uint32_t w, std::uint32_t h, BYTE* pBuff);

    private:
        ID2D1Factory* m_pFactory{ nullptr };
    };
}

namespace app
{
    class D3DContext
    {
    public:
        struct FrameContext
        {
            ID3D12CommandAllocator* CommandAllocator;
            UINT64                  FenceValue;
        };

    public:
        D3DContext() = default;
        ~D3DContext();

    public:
        bool Init(HWND hWnd);
        //bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();
        void WaitForLastSubmittedFrame();
        FrameContext* WaitForNextFrameResources();

        bool ResizeBuffer(std::uint32_t w, std::uint32_t h);
        void WaitFence();
        IDXGISurface* GetBackBuffer();

    private:
        static constexpr auto         NUM_FRAMES_IN_FLIGHT = 3;
        FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
        UINT                         g_frameIndex = 0;

        static constexpr auto        NUM_BACK_BUFFERS = 3;
        ID3D12Device* g_pd3dDevice = nullptr;
        ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
        ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
        ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
        ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
        ID3D12Fence* g_fence = nullptr;
        HANDLE                       g_fenceEvent = nullptr;
        UINT64                       g_fenceLastSignaledValue = 0;
        IDXGISwapChain3* g_pSwapChain = nullptr;
        HANDLE                       g_hSwapChainWaitableObject = nullptr;
        ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
        D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
    };
}
