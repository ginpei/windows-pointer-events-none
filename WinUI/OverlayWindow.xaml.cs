using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.UI.Xaml.Shapes;
using System;
using System.Runtime.InteropServices;
using Windows.Graphics;
using WinRT.Interop;

namespace WinUI
{
    public sealed partial class OverlayWindow : Window
    {
        private const int WS_EX_TRANSPARENT = 0x00000020;
        private const int WS_EX_LAYERED = 0x00080000;
        private const int GWL_EXSTYLE = -20;
        private const int LWA_ALPHA = 0x00000002;

        [DllImport("user32.dll")]
        private static extern int GetWindowLong(IntPtr hwnd, int index);

        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hwnd, int index, int newStyle);

        [DllImport("user32.dll")]
        private static extern bool SetLayeredWindowAttributes(IntPtr hwnd, uint crKey, byte bAlpha, uint dwFlags);

        private AppWindow? _appWindow;

        public OverlayWindow()
        {
            InitializeComponent();
            InitializeWindow();
            
            if (Content is FrameworkElement content)
            {
                content.Loaded += Content_Loaded;
            }
        }

        private void InitializeWindow()
        {
            var hwnd = WindowNative.GetWindowHandle(this);
            var windowId = Win32Interop.GetWindowIdFromWindow(hwnd);
            _appWindow = AppWindow.GetFromWindowId(windowId);

            if (_appWindow != null)
            {
                var displayArea = DisplayArea.Primary;
                _appWindow.MoveAndResize(new RectInt32(
                    0, 
                    0, 
                    displayArea.WorkArea.Width, 
                    displayArea.WorkArea.Height));

                var presenter = _appWindow.Presenter as OverlappedPresenter;
                if (presenter != null)
                {
                    presenter.IsAlwaysOnTop = true;
                    presenter.IsMaximizable = false;
                    presenter.IsMinimizable = false;
                    presenter.IsResizable = false;
                    presenter.SetBorderAndTitleBar(false, false);
                }
            }

            SystemBackdrop = null;
            
            // Make window layered for transparency support
            MakeWindowTransparent(hwnd);
        }

        private void MakeWindowTransparent(IntPtr hwnd)
        {
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_LAYERED);
            
            // Set initial opacity to 50% (128 out of 255)
            SetLayeredWindowAttributes(hwnd, 0, 128, LWA_ALPHA);
        }

        private void Content_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateClockHands();
            MakeWindowClickThrough();
            StartFadeOut();
        }

        private void StartFadeOut()
        {
            var hwnd = WindowNative.GetWindowHandle(this);
            
            var startTime = DateTime.Now;
            var duration = TimeSpan.FromMilliseconds(3000);
            var startOpacity = 128; // 50%
            var endOpacity = 0;

            var dispatcherQueue = Microsoft.UI.Dispatching.DispatcherQueue.GetForCurrentThread();
            var timer = dispatcherQueue.CreateTimer();
            
            timer.Interval = TimeSpan.FromMilliseconds(16); // ~60fps
            timer.Tick += (s, e) =>
            {
                var elapsed = DateTime.Now - startTime;
                if (elapsed >= duration)
                {
                    SetLayeredWindowAttributes(hwnd, 0, (byte)endOpacity, LWA_ALPHA);
                    timer.Stop();
                    Close();
                }
                else
                {
                    var progress = elapsed.TotalMilliseconds / duration.TotalMilliseconds;
                    var currentOpacity = startOpacity - (int)((startOpacity - endOpacity) * progress);
                    SetLayeredWindowAttributes(hwnd, 0, (byte)currentOpacity, LWA_ALPHA);
                }
            };
            
            timer.Start();
        }

        private void UpdateViewboxSize()
        {
            if (this.Content is Grid grid && grid.Children.Count > 0 && grid.Children[0] is Viewbox vb)
            {
                double minDimension = Math.Min(this.Bounds.Width, this.Bounds.Height);
                double targetSize = minDimension * 0.8;
                vb.Width = targetSize;
                vb.Height = targetSize;
            }
        }

        private void UpdateClockHands()
        {
            var now = DateTime.Now;
            var hourAngle = (now.Hour % 12 + now.Minute / 60.0) * 30;
            var minuteAngle = now.Minute * 6;

            if (HourHand != null)
            {
                var hourTransform = new RotateTransform
                {
                    Angle = hourAngle,
                    CenterX = 50,
                    CenterY = 50
                };
                HourHand.RenderTransform = hourTransform;
            }

            if (MinuteHand != null)
            {
                var minuteTransform = new RotateTransform
                {
                    Angle = minuteAngle,
                    CenterX = 50,
                    CenterY = 50
                };
                MinuteHand.RenderTransform = minuteTransform;
            }
        }

        private void MakeWindowClickThrough()
        {
            var hwnd = WindowNative.GetWindowHandle(this);
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
        }

        public void DisableClickThrough()
        {
            var hwnd = WindowNative.GetWindowHandle(this);
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle & ~WS_EX_TRANSPARENT);
        }
    }
}
