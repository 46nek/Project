// 必要なヘッダーファイルをインクルードします
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

// グローバル変数
// これらはプログラム全体で使うDirectX11の重要なオブジェクトです
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
HWND                    g_hWnd = nullptr; // ウィンドウハンドル

// 関数のプロトタイプ宣言（これからこういう関数を作りますよ、という宣言）
HRESULT InitD3D();
void Cleanup();
void Render();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Windowsアプリケーションのエントリーポイント（ここからプログラムが始まります）
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // 1. ウィンドウクラスの登録
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"MyGameWindowClass", NULL };
    RegisterClassEx(&wc);

    // 2. ウィンドウの作成
    // ゲーム画面のサイズをここで決めます
    int screenWidth = 1280;
    int screenHeight = 720;
    g_hWnd = CreateWindow(L"MyGameWindowClass", L"DirectX11 Game", WS_OVERLAPPEDWINDOW, 100, 100, screenWidth, screenHeight, NULL, NULL, wc.hInstance, NULL);

    // 3. DirectX11の初期化
    if (SUCCEEDED(InitD3D()))
    {
        // 4. ウィンドウの表示
        ShowWindow(g_hWnd, nCmdShow);
        UpdateWindow(g_hWnd);

        // 5. メインループ（ゲームループ）
        // このループがゲームの本体です。ウィンドウが閉じられるまで回り続けます。
        MSG msg = { 0 };
        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // 描画処理を呼び出します
                Render();
            }
        }
    }

    // 6. クリーンアップ
    // プログラム終了時に後片付けをします
    Cleanup();

    return 0;
}

// DirectX11の初期化を行う関数
HRESULT InitD3D()
{
    HRESULT hr = S_OK;

    // ウィンドウのクライアント領域のサイズを取得
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    // スワップチェーンとD3Dデバイスの作成
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; // ダブルバッファリングを使わない場合は1
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 一般的な32ビットカラー
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE; // ウィンドウモードで起動

    hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        NULL,
        &g_pImmediateContext
    );
    if (FAILED(hr)) return hr;

    // レンダーターゲットビューの作成
    // 描画する対象（バックバッファ）を取得します
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) return hr;

    // バックバッファを描画ターゲットとして設定します
    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
    pBackBuffer->Release(); // 取得したバックバッファはもう不要なので解放
    if (FAILED(hr)) return hr;

    // 描画ターゲットをデバイスコンテキストに設定
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

    // ビューポートの設定
    // 描画する領域をウィンドウ全体に設定します
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    return S_OK;
}

// 描画処理を行う関数
void Render()
{
    // 背景色をコーンフラワーブルーで塗りつぶします
    float ClearColor[4] = { 0.39f, 0.58f, 0.93f, 1.0f }; // RGBA
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

    // 描画したものを画面に表示します
    g_pSwapChain->Present(0, 0);
}

// リソースを解放する関数
void Cleanup()
{
    // 作成したオブジェクトを逆の順序で解放していきます
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
}

// ウィンドウのイベントを処理する関数（ウィンドウプロシージャ）
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY: // ウィンドウが閉じられたときの処理
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}