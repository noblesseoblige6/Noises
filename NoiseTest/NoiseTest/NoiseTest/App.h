#pragma once
#include <iostream>
#include <memory>

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
        bool MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
        bool m_isResized{ false };
        bool m_beginResize{ false };

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
        Imgui(HWND hWnd, ID3D11Device* pDevice, std::uint32_t buffeFrames, ID3D11DeviceContext* pContext);
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
        D3DContext() = default;
        ~D3DContext();

    public:
        bool Init(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();

        bool ResizeBuffer(std::uint32_t w, std::uint32_t h);
        void WaitFence();

        IDXGISurface* GetBackBuffer();
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
