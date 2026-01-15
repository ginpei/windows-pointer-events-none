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
    int m_fadeoutCounter;
    BYTE m_currentAlpha;

    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void DrawClock(HDC hdc, RECT& clientRect);
    void DrawClockHand(HDC hdc, int centerX, int centerY, double angle, int length, int thickness);
    void MakeWindowClickThrough();

public:
    OverlayWindow(HINSTANCE hInstance);
    ~OverlayWindow();
    bool Create();
    void Show();
};
