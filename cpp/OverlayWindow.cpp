#include "OverlayWindow.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const wchar_t* OVERLAY_CLASS_NAME = L"OverlayWindowClass";

OverlayWindow::OverlayWindow(HINSTANCE hInstance)
    : m_hWnd(nullptr)
    , m_hInstance(hInstance)
    , m_fadeoutCounter(0)
    , m_currentAlpha(255)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_clockSize(0)
    , m_hdcMem(nullptr)
    , m_hBitmap(nullptr)
    , m_hOldBitmap(nullptr)
    , m_pBits(nullptr)
{
}

OverlayWindow::~OverlayWindow()
{
    if (m_hdcMem)
    {
        if (m_hOldBitmap)
        {
            SelectObject(m_hdcMem, m_hOldBitmap);
        }
        if (m_hBitmap)
        {
            DeleteObject(m_hBitmap);
        }
        DeleteDC(m_hdcMem);
    }
    
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}

bool OverlayWindow::Create()
{
    static bool s_classRegistered = false;
    
    if (!s_classRegistered)
    {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProcStatic;
        wcex.hInstance = m_hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wcex.lpszClassName = OVERLAY_CLASS_NAME;

        if (!RegisterClassExW(&wcex))
        {
            return false;
        }
        
        s_classRegistered = true;
    }

    m_screenWidth = GetSystemMetrics(SM_CXSCREEN);
    m_screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    int minDimension = min(m_screenWidth, m_screenHeight);
    m_clockSize = static_cast<int>(minDimension * 0.8);

    m_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        OVERLAY_CLASS_NAME,
        L"Overlay",
        WS_POPUP,
        0, 0, m_screenWidth, m_screenHeight,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hWnd)
    {
        return false;
    }

    MakeWindowClickThrough();
    CreateClockBitmap();
    
    return true;
}

void OverlayWindow::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
    UpdateWindowDisplay();
}

void OverlayWindow::MakeWindowClickThrough()
{
    LONG exStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    SetWindowLong(m_hWnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
}

LRESULT CALLBACK OverlayWindow::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    OverlayWindow* pThis = nullptr;

    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<OverlayWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<OverlayWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->WndProc(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT OverlayWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, TIMER_ID, 1000, nullptr);
        SetTimer(hWnd, FADEOUT_TIMER_ID, 30, nullptr);
        m_fadeoutCounter = 0;
        m_currentAlpha = 255;
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            CreateClockBitmap();
            UpdateWindowDisplay();
        }
        else if (wParam == FADEOUT_TIMER_ID)
        {
            m_fadeoutCounter++;
            if (m_fadeoutCounter >= 100)
            {
                DestroyWindow(hWnd);
            }
            else
            {
                m_currentAlpha = static_cast<BYTE>(255 * (100 - m_fadeoutCounter) / 100);
                UpdateWindowDisplay();
            }
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ID);
        KillTimer(hWnd, FADEOUT_TIMER_ID);
        PostMessage(hWnd, WM_CLEANUP, 0, 0);
        return 0;
        
    case WM_CLEANUP:
        delete this;
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void OverlayWindow::CreateClockBitmap()
{
    HDC hdcScreen = GetDC(nullptr);
    if (!hdcScreen)
        return;
    
    if (!m_hdcMem)
    {
        m_hdcMem = CreateCompatibleDC(hdcScreen);
        if (!m_hdcMem)
        {
            ReleaseDC(nullptr, hdcScreen);
            return;
        }
        
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_clockSize;
        bmi.bmiHeader.biHeight = -m_clockSize;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        m_hBitmap = CreateDIBSection(m_hdcMem, &bmi, DIB_RGB_COLORS, (void**)&m_pBits, nullptr, 0);
        if (!m_hBitmap || !m_pBits)
        {
            DeleteDC(m_hdcMem);
            m_hdcMem = nullptr;
            ReleaseDC(nullptr, hdcScreen);
            return;
        }
        
        m_hOldBitmap = (HBITMAP)SelectObject(m_hdcMem, m_hBitmap);
    }

    Graphics graphics(m_hdcMem);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.Clear(Color(0, 0, 0, 0));

    float centerX = m_clockSize / 2.0f;
    float centerY = m_clockSize / 2.0f;
    float diameter = m_clockSize - 4.0f;
    float margin = 2.0f;

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    graphics.FillEllipse(&whiteBrush, margin, margin, diameter, diameter);

    Pen blackPen(Color(255, 0, 0, 0), 3.0f);
    blackPen.SetStartCap(LineCapRound);
    blackPen.SetEndCap(LineCapRound);
    graphics.DrawEllipse(&blackPen, margin + 1.5f, margin + 1.5f, diameter - 3.0f, diameter - 3.0f);

    SYSTEMTIME st;
    GetLocalTime(&st);

    double hourAngle = ((st.wHour % 12) + st.wMinute / 60.0) * 30.0;
    double minuteAngle = st.wMinute * 6.0;

    double hourRadian = (hourAngle - 90.0) * M_PI / 180.0;
    float hourLength = (diameter / 2.0f) * 0.5f;
    float hourEndX = centerX + hourLength * static_cast<float>(cos(hourRadian));
    float hourEndY = centerY + hourLength * static_cast<float>(sin(hourRadian));

    Pen hourPen(Color(255, 0, 0, 0), 4.0f);
    hourPen.SetStartCap(LineCapRound);
    hourPen.SetEndCap(LineCapRound);
    graphics.DrawLine(&hourPen, centerX, centerY, hourEndX, hourEndY);

    double minuteRadian = (minuteAngle - 90.0) * M_PI / 180.0;
    float minuteLength = (diameter / 2.0f) * 0.7f;
    float minuteEndX = centerX + minuteLength * static_cast<float>(cos(minuteRadian));
    float minuteEndY = centerY + minuteLength * static_cast<float>(sin(minuteRadian));

    Pen minutePen(Color(255, 0, 0, 0), 2.0f);
    minutePen.SetStartCap(LineCapRound);
    minutePen.SetEndCap(LineCapRound);
    graphics.DrawLine(&minutePen, centerX, centerY, minuteEndX, minuteEndY);

    SolidBrush centerBrush(Color(255, 0, 0, 0));
    graphics.FillEllipse(&centerBrush, centerX - 4.0f, centerY - 4.0f, 8.0f, 8.0f);

    ApplyCircularAlphaMask();

    ReleaseDC(nullptr, hdcScreen);
}

void OverlayWindow::ApplyCircularAlphaMask()
{
    if (!m_pBits)
        return;

    float centerX = m_clockSize / 2.0f;
    float centerY = m_clockSize / 2.0f;
    float outerRadius = m_clockSize / 2.0f;
    float innerRadius = outerRadius - 3.0f;
    float innerRadiusSquared = innerRadius * innerRadius;
    float outerRadiusSquared = outerRadius * outerRadius;

    for (int y = 0; y < m_clockSize; y++)
    {
        for (int x = 0; x < m_clockSize; x++)
        {
            float dx = x + 0.5f - centerX;
            float dy = y + 0.5f - centerY;
            float distSquared = dx * dx + dy * dy;
            
            int index = (y * m_clockSize + x) * 4;
            BYTE b = m_pBits[index + 0];
            BYTE g = m_pBits[index + 1];
            BYTE r = m_pBits[index + 2];
            BYTE a = m_pBits[index + 3];
            
            if (distSquared >= outerRadiusSquared)
            {
                m_pBits[index + 0] = 0;
                m_pBits[index + 1] = 0;
                m_pBits[index + 2] = 0;
                m_pBits[index + 3] = 0;
            }
            else if (distSquared >= innerRadiusSquared)
            {
                float dist = sqrt(distSquared);
                float alpha = (outerRadius - dist) / (outerRadius - innerRadius);
                alpha = max(0.0f, min(1.0f, alpha));
                
                BYTE newAlpha = static_cast<BYTE>(a * alpha);
                m_pBits[index + 0] = static_cast<BYTE>(b * newAlpha / 255);
                m_pBits[index + 1] = static_cast<BYTE>(g * newAlpha / 255);
                m_pBits[index + 2] = static_cast<BYTE>(r * newAlpha / 255);
                m_pBits[index + 3] = newAlpha;
            }
            else
            {
                m_pBits[index + 0] = static_cast<BYTE>(b * a / 255);
                m_pBits[index + 1] = static_cast<BYTE>(g * a / 255);
                m_pBits[index + 2] = static_cast<BYTE>(r * a / 255);
            }
        }
    }
}

void OverlayWindow::UpdateWindowDisplay()
{
    if (!m_hdcMem || !m_hBitmap)
        return;

    HDC hdcScreen = GetDC(nullptr);
    if (!hdcScreen)
        return;

    POINT ptSrc = { 0, 0 };
    POINT ptDest = { (m_screenWidth - m_clockSize) / 2, (m_screenHeight - m_clockSize) / 2 };
    SIZE sizeWnd = { m_clockSize, m_clockSize };
    
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = m_currentAlpha;
    blend.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(m_hWnd, hdcScreen, &ptDest, &sizeWnd, m_hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    ReleaseDC(nullptr, hdcScreen);
}
