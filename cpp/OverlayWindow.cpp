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
    , m_currentAlpha(128)
{
}

OverlayWindow::~OverlayWindow()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}

bool OverlayWindow::Create()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcStatic;
    wcex.hInstance = m_hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcex.lpszClassName = OVERLAY_CLASS_NAME;

    RegisterClassExW(&wcex);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        OVERLAY_CLASS_NAME,
        L"Overlay",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hWnd)
    {
        return false;
    }

    SetLayeredWindowAttributes(m_hWnd, 0, 128, LWA_ALPHA);
    MakeWindowClickThrough();
    
    return true;
}

void OverlayWindow::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
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
        m_currentAlpha = 128;
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            DrawClock(hdc, clientRect);
            
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            InvalidateRect(hWnd, nullptr, FALSE);
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
                m_currentAlpha = static_cast<BYTE>(128 * (100 - m_fadeoutCounter) / 100);
                SetLayeredWindowAttributes(hWnd, 0, m_currentAlpha, LWA_ALPHA);
            }
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ID);
        KillTimer(hWnd, FADEOUT_TIMER_ID);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void OverlayWindow::DrawClock(HDC hdc, RECT& clientRect)
{
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    int minDimension = min(width, height);
    int clockSize = static_cast<int>(minDimension * 0.8);
    int centerX = width / 2;
    int centerY = height / 2;
    int radius = clockSize / 2;

    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    Pen blackPen(Color(255, 0, 0, 0), 3.0f);
    blackPen.SetStartCap(LineCapRound);
    blackPen.SetEndCap(LineCapRound);

    graphics.FillEllipse(&whiteBrush, centerX - radius, centerY - radius, radius * 2, radius * 2);
    graphics.DrawEllipse(&blackPen, centerX - radius, centerY - radius, radius * 2, radius * 2);

    SYSTEMTIME st;
    GetLocalTime(&st);

    double hourAngle = ((st.wHour % 12) + st.wMinute / 60.0) * 30.0;
    double minuteAngle = st.wMinute * 6.0;

    DrawClockHand(hdc, centerX, centerY, hourAngle, static_cast<int>(radius * 0.5), 4);
    DrawClockHand(hdc, centerX, centerY, minuteAngle, static_cast<int>(radius * 0.7), 2);

    SolidBrush centerBrush(Color(255, 0, 0, 0));
    graphics.FillEllipse(&centerBrush, centerX - 4, centerY - 4, 8, 8);
}

void OverlayWindow::DrawClockHand(HDC hdc, int centerX, int centerY, double angle, int length, int thickness)
{
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    double radian = (angle - 90.0) * M_PI / 180.0;
    int endX = centerX + static_cast<int>(length * cos(radian));
    int endY = centerY + static_cast<int>(length * sin(radian));

    Pen pen(Color(255, 0, 0, 0), static_cast<REAL>(thickness));
    pen.SetStartCap(LineCapRound);
    pen.SetEndCap(LineCapRound);

    graphics.DrawLine(&pen, centerX, centerY, endX, endY);
}
