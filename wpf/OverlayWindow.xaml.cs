using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Shapes;

namespace wpf
{
    public partial class OverlayWindow : Window
    {
        private const int WS_EX_TRANSPARENT = 0x00000020;
        private const int GWL_EXSTYLE = -20;

        [DllImport("user32.dll")]
        private static extern int GetWindowLong(IntPtr hwnd, int index);

        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hwnd, int index, int newStyle);

        public OverlayWindow()
        {
            InitializeComponent();
            Topmost = true;
            Opacity = 0.5;
            Loaded += OverlayWindow_Loaded;
            SizeChanged += OverlayWindow_SizeChanged;
        }

        private void OverlayWindow_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateClockHands();
            MakeWindowClickThrough();
        }

        private void OverlayWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            UpdateViewboxSize();
        }

        private void UpdateViewboxSize()
        {
            var viewbox = this.Content as Grid;
            if (viewbox?.Children[0] is Viewbox vb)
            {
                double minDimension = Math.Min(ActualWidth, ActualHeight);
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

            var hourHand = FindName("HourHand") as Line;
            var minuteHand = FindName("MinuteHand") as Line;

            if (hourHand != null)
            {
                var hourTransform = new RotateTransform(hourAngle, 50, 50);
                hourHand.RenderTransform = hourTransform;
            }

            if (minuteHand != null)
            {
                var minuteTransform = new RotateTransform(minuteAngle, 50, 50);
                minuteHand.RenderTransform = minuteTransform;
            }
        }

        private void MakeWindowClickThrough()
        {
            var hwnd = new WindowInteropHelper(this).Handle;
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
        }

        public void DisableClickThrough()
        {
            var hwnd = new WindowInteropHelper(this).Handle;
            int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle & ~WS_EX_TRANSPARENT);
        }
    }
}
