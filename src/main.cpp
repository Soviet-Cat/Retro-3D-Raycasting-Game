#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <windows.h>

void debugMsg()
{
    std::cout << "Here \n";
}

void coutLastError()
{
    DWORD dwError = GetLastError();
    if (dwError == 0)
    {
        return;
    }

    LPVOID lpMsgBuf;
    DWORD dwSize = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr
    );
    if (dwSize)
    {
        std::wcout << L"Error " << dwError << L": " << (LPWSTR)lpMsgBuf << L"\n";
        LocalFree(lpMsgBuf);
    } else
    {
        std::cout << "Failed to get error message for code: " << dwError << "\n";
    }
}

struct Window
{
    HINSTANCE hInst;
    HWND hWnd;
    LPCSTR lpClassName = "Retro-3D-Raycasting-Game-Window";
    LPCSTR lpTitle = "Retro-3D-Raycasting-Game-Window";
    UINT uWidth = 640;
    UINT uHeight = 360;
    WNDCLASSEX wndClass;
    MSG msg;
    BOOLEAN bClose = false;
    LPVOID pData = nullptr;
};

LRESULT CALLBACK Window_process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT Window_register(Window* pWnd, HINSTANCE hInst)
{
    // Setup WNDCLASSEX
    pWnd->wndClass = {};
    pWnd->wndClass.cbSize = sizeof(pWnd->wndClass);
    pWnd->wndClass.style = CS_HREDRAW | CS_VREDRAW;
    pWnd->wndClass.lpfnWndProc = Window_process;
    pWnd->wndClass.cbClsExtra = 0;
    pWnd->wndClass.cbWndExtra = 0;
    pWnd->wndClass.hInstance = hInst;
    pWnd->wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    pWnd->wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    pWnd->wndClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    pWnd->wndClass.lpszClassName = pWnd->lpClassName;
    pWnd->wndClass.lpszMenuName = nullptr;
    pWnd->wndClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    // Register Class
    if (RegisterClassEx(&pWnd->wndClass) == 0)
    {
        coutLastError();
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Window_unregister(Window* pWnd)
{
    if (UnregisterClass(pWnd->lpClassName, pWnd->hInst) == 0)
    {
        coutLastError();
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Window_create(Window* pWnd, int iCmdShow)
{
    // Create window
    pWnd->hWnd = CreateWindowEx(
        0,
        pWnd->lpClassName, pWnd->lpTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        pWnd->uWidth, pWnd->uHeight,
        nullptr, nullptr, pWnd->hInst, nullptr
    );

    if (pWnd->hWnd == nullptr)
    {
        coutLastError();
        return E_FAIL;
    }

    //* Connect pWnd to hWnd through GWLP_USERDATA
    SetWindowLongPtr(pWnd->hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

    //! Make window visible. DO NOT REMOVE THIS
    ShowWindow(pWnd->hWnd, iCmdShow);

    return S_OK;
}

void Window_message(Window* pWnd)
{
    if (PeekMessage(&pWnd->msg, pWnd->hWnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&pWnd->msg);
        DispatchMessage(&pWnd->msg);
    }
}

struct Buffer
{
    HWND hWnd = nullptr;
    UINT uWidth, uHeight;
    HBITMAP hBm;
    BITMAPINFO bmi;
    UINT32* data;
};

HRESULT Buffer_create(Buffer* pBuf)
{
    HDC hDC = GetDC(pBuf->hWnd);

    if (hDC == nullptr)
    {
        coutLastError();
        return E_FAIL;
    }

    pBuf->bmi = {};
    pBuf->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pBuf->bmi.bmiHeader.biWidth = pBuf->uWidth;
    pBuf->bmi.bmiHeader.biHeight = -pBuf->uHeight;
    pBuf->bmi.bmiHeader.biPlanes = 1;
    pBuf->bmi.bmiHeader.biBitCount = 32;
    pBuf->bmi.bmiHeader.biCompression = BI_RGB;

    pBuf->hBm = CreateDIBSection(hDC, &pBuf->bmi, DIB_RGB_COLORS, reinterpret_cast<LPVOID*>(&pBuf->data), nullptr, 0);
    if (pBuf->hBm == nullptr)
    {
        coutLastError();
        return E_FAIL;
    }

    if (ReleaseDC(pBuf->hWnd, hDC) == 0)
    {
        coutLastError();
        return E_FAIL;
    }

    return S_OK;
}

UINT Buffer_size(Buffer* pBuf)
{
    return pBuf->uWidth * pBuf->uHeight;
}

HRESULT Buffer_destroy(Buffer* pBuf)
{
    if (pBuf == nullptr)
    {
        return E_FAIL;
    }

    if (pBuf->hBm)
    {
        if (DeleteObject(pBuf->hBm) == 0)
        {
            coutLastError();
            return E_FAIL;
        }
        pBuf->hBm = nullptr;
    }

    return S_OK;
}

struct Renderer
{
    HWND hWnd;
    UINT uWidth = 640;
    UINT uHeight = 360;
    Buffer* pBufFront;
    Buffer* pBufBack;
    float fFps = 60.0f;
    UINT uTimerID = 1;
};

HRESULT Renderer_create(Renderer* pRenderer)
{
    pRenderer->pBufFront = new Buffer();
    pRenderer->pBufFront->hWnd = pRenderer->hWnd;
    pRenderer->pBufFront->uWidth = pRenderer->uWidth;
    pRenderer->pBufFront->uHeight = pRenderer->uHeight;
    if (FAILED(Buffer_create(pRenderer->pBufFront)))
    {
        return E_FAIL;
    }

    pRenderer->pBufBack = new Buffer();
    pRenderer->pBufBack->hWnd = pRenderer->hWnd;
    pRenderer->pBufBack->uWidth = pRenderer->uWidth;
    pRenderer->pBufBack->uHeight = pRenderer->uHeight;
    if (FAILED(Buffer_create(pRenderer->pBufBack)))
    {
        return E_FAIL;
    }

    RECT rClient;
    GetClientRect(pRenderer->hWnd, &rClient);
    SetTimer(pRenderer->hWnd, pRenderer->uTimerID, 1000 / pRenderer->fFps, nullptr);

    return S_OK;
}

UINT Renderer_size(Renderer* pRenderer)
{
    return pRenderer->uWidth * pRenderer->uHeight;
}

void Renderer_swap(Renderer* pRenderer)
{
    std::swap(pRenderer->pBufFront, pRenderer->pBufBack);
}

void Renderer_clear(Renderer* pRenderer)
{
    std::fill(pRenderer->pBufBack->data, pRenderer->pBufBack->data + Buffer_size(pRenderer->pBufBack), 0);
}

void Renderer_set(Renderer* pRenderer, UINT uX, UINT uY, UINT32 uPixel)
{
    pRenderer->pBufBack->data[uY * pRenderer->pBufBack->uWidth + uX] = uPixel;
}

struct Window_process_t
{
    Renderer* pRenderer;
};

LRESULT CALLBACK Window_process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    Window_process_t* pData = nullptr;
    if (pWnd)
    {
        pData = reinterpret_cast<Window_process_t*>(pWnd->pData);
    }

    switch (uMsg)
    {
        case WM_DESTROY:
        {
            if (pWnd)
            {
                pWnd->bClose = true;
            }
            PostQuitMessage(0);
            break;
        }
        case WM_TIMER:
        {
            if (pData)
            {
                Renderer_swap(pData->pRenderer);
                Renderer_clear(pData->pRenderer);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            break;
        }
        case WM_PAINT:
        {
            if (pData && pData->pRenderer->pBufFront->hBm)
            {
                PAINTSTRUCT ps;
                HDC hDC = BeginPaint(hWnd, &ps);
                HDC hDCCompatible = CreateCompatibleDC(hDC);
                auto bm = SelectObject(hDCCompatible, pData->pRenderer->pBufFront->hBm);
                BitBlt(hDC, 0, 0, pData->pRenderer->pBufFront->uWidth, pData->pRenderer->pBufFront->uHeight, hDCCompatible, 0, 0, SRCCOPY);
                SelectObject(hDC, bm);
                DeleteDC(hDCCompatible);
                EndPaint(hWnd, &ps);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    //* Default cases (undefined behaviour)
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void cleanup(Window* pWnd, Renderer* pRenderer)
{
    if (pRenderer)
    {
        if (SUCCEEDED(Buffer_destroy(pRenderer->pBufFront)))
        {
            delete pRenderer->pBufFront;
        }
        if (SUCCEEDED(Buffer_destroy(pRenderer->pBufBack)))
        {
            delete pRenderer->pBufBack;
        }
        delete pRenderer;
    }

    auto pData = reinterpret_cast<Window_process_t*>(pWnd->pData);
    if (pData)
    {
        delete pData;
    }
    Window_unregister(pWnd);
    delete pWnd;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int iCmdShow)
{
    #ifdef ALLOC_CONSOLE
    AllocConsole();
    FILE* fConsole;
    freopen_s(&fConsole, "CONOUT$", "w", stdout);
    freopen_s(&fConsole, "CONIN$", "r", stdin);
    #endif

    Window* pWnd = new Window();
    pWnd->hInst = hInst;
    pWnd->pData = reinterpret_cast<LPVOID>(new Window_process_t());
    auto pData = reinterpret_cast<Window_process_t*>(pWnd->pData);

    // Register and create windows
    if (FAILED(Window_register(pWnd, hInst)))
    {
        cleanup(pWnd, nullptr);
        return 1;
    }
    if (FAILED(Window_create(pWnd, iCmdShow)))
    {
        cleanup(pWnd, nullptr);
        return 1;
    }

    // Create and initialize renderer
    Renderer* pRenderer = new Renderer();
    pRenderer->hWnd = pWnd->hWnd;
    pData->pRenderer = pRenderer;

    if (FAILED(Renderer_create(pRenderer)))
    {
        cleanup(pWnd, pRenderer);
        return 1;
    }

    while (!pWnd->bClose)
    {
        for (UINT y = 0; y < pRenderer->pBufFront->uHeight; y++)
        {
            for (UINT x = 0; x < pRenderer->pBufFront->uWidth; x++)
            {
                UINT gradient = (x * 255) / pRenderer->pBufFront->uWidth;
                UINT color = (0xff << 24) | (gradient << 16) | (gradient << 8) | gradient;
                Renderer_set(pRenderer, x, y, color);
            }
        }
        Window_message(pWnd);
    }

    cleanup(pWnd, pRenderer);

    #ifdef ALLOC_CONSOLE
    system("pause");
    #endif

    return 0;
}