using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace wpf
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private OverlayWindow? overlayWindow;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void ShowOverlay_Click(object sender, RoutedEventArgs e)
        {
            if (overlayWindow == null || !overlayWindow.IsVisible)
            {
                overlayWindow = new OverlayWindow();
                overlayWindow.Show();
            }
        }

        private void HideOverlay_Click(object sender, RoutedEventArgs e)
        {
            overlayWindow?.Close();
            overlayWindow = null;
        }
    }
}