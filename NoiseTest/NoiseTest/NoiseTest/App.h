#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <future>

#include <wtypes.h>
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_4.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11.lib")

#define DX12_ENABLE_DEBUG_LAYER

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace app
{
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

    private:
        bool Init();
        void Update();
        void Render();
        void SwapFrame();

        bool InitD3D();
        void UpdateWindowSize();
        void UpdateNoiseTex();

        void UpdateNoise();
        void UpdateNoiseAsync();

        void UpdateSize();

    private:
        HWND          m_hWnd;
        HINSTANCE     m_hInst;

        bool m_isInit{ false };
        bool m_isResized{ false };
        bool m_beginResize{ false };

        using Rectf = std::tuple<std::float_t, std::float_t, std::float_t, std::float_t>;
        Rectf m_clientSize;
        Rectf m_propertySize;
        Rectf m_previewSize;

        using Size2f = std::pair<std::float_t, std::float_t>;
        Size2f m_texSize;

        std::uint32_t m_width{ 1024 };
        std::uint32_t m_height{ 1024 };

        std::int32_t m_size{ 512 };

        std::int32_t m_noiseType{ 0 };
        std::int32_t m_octave{ 1 };
        std::float_t m_frequency{ 1 / 32.f };
        std::float_t m_persistence{ 0.5f };

        std::unique_ptr<Imgui> m_pImgui{ nullptr };

        std::unique_ptr <D3DContext> m_p3DContext{ nullptr };

        UINT8* m_pTexBuffer = nullptr;
        ID3D11ShaderResourceView* m_pNoiseTex{ nullptr };

        bool m_keptChange = false;
        std::future<void> m_future;
        std::mutex m_mtx;
    };
}

namespace app
{
    class Imgui
    {
    public:
        Imgui(HWND hWnd, ID3D11Device* pDevice, std::uint32_t buffeFrames, ID3D11DeviceContext* pContext);
        ~Imgui();

    public:
        void Begin();
    };
}

namespace app
{
    class D3DContext
    {
    public:
        D3DContext() = default;
        ~D3DContext();

    public:
        bool Init(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();

        bool ResizeBuffer(std::uint32_t w, std::uint32_t h);
        void WaitFence();

        ID3D11ShaderResourceView* CreateTexture(std::uint32_t w, std::uint32_t h, UINT8* pData);
        ID3D11Device* GetDevice();
        ID3D11DeviceContext* GetDeviceContext();
        ID3D11RenderTargetView* pRTV();

    private:
        ID3D11Device*           m_pd3dDevice = nullptr;
        ID3D11DeviceContext*    m_pd3dDeviceContext = nullptr;
        IDXGISwapChain*         m_pSwapChain = nullptr;
        ID3D11RenderTargetView* m_pRTV = nullptr;
    };
}
