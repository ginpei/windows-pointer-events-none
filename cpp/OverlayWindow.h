#pragma once
#include "framework.h"
#include <cmath>

class OverlayWindow
{
private:
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    static constexpr int TIMER_ID = 1;
    static constexpr int FADEOUT_TIMER_ID = 2;
    static constexpr int WM_CLEANUP = WM_USER + 1;
    int m_fadeoutCounter;
    BYTE m_currentAlpha;
    int m_screenWidth;
    int m_screenHeight;
    int m_clockSize;
    HDC m_hdcMem;
    HBITMAP m_hBitmap;
    HBITMAP m_hOldBitmap;
    BYTE* m_pBits;

    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CreateClockBitmap();
    void ApplyCircularAlphaMask();
    void UpdateWindowDisplay();
    void MakeWindowClickThrough();

public:
    OverlayWindow(HINSTANCE hInstance);
    ~OverlayWindow();
    bool Create();
    void Show();
};
