# Windows Pointer Events None - Click-Through Window Demonstration

A demonstration project showcasing click-through window implementations across different Windows UI frameworks (C++/Win32, WPF, and WinUI 3).

## Overview

This repository contains three separate implementations of a click-through overlay window feature. Each implementation demonstrates how to create a transparent, click-through window that displays an animated clock overlay using different Windows development technologies.

## Projects

### 1. C++ / Win32 (`cpp/`)

A native Windows application using Win32 API and GDI+.

**Technologies:**
- Win32 API
- GDI+ for graphics rendering
- Native C++ (no managed code)

**Features:**
- Pure Win32 window management
- GDI+ based clock rendering
- Layered window with alpha blending
- Click-through using `WS_EX_TRANSPARENT` window style
- Custom fade-out animation

**Key Files:**
- `cpp.cpp` - Main application entry point
- `OverlayWindow.cpp` / `OverlayWindow.h` - Overlay window implementation

**Build Requirements:**
- Visual Studio 2019 or later
- Windows SDK
- C++ Desktop Development workload

### 2. WPF (`wpf/`)

A managed application using Windows Presentation Foundation.

**Technologies:**
- .NET 10
- WPF (Windows Presentation Foundation)
- XAML for UI definition

**Features:**
- XAML-based declarative UI
- WPF graphics rendering
- Window interop with Win32 for click-through functionality
- Storyboard-based fade-out animation
- Modern .NET with nullable reference types enabled

**Key Files:**
- `MainWindow.xaml` / `MainWindow.xaml.cs` - Main application window
- `OverlayWindow.xaml` / `OverlayWindow.xaml.cs` - Overlay window implementation

**Build Requirements:**
- Visual Studio 2022 or later
- .NET 10 SDK
- WPF workload

### 3. WinUI 3 (`WinUI/`)

A modern Windows application using the Windows App SDK.

**Technologies:**
- .NET 8
- WinUI 3 (Windows App SDK 1.8)
- XAML for UI definition

**Features:**
- Modern WinUI 3 controls and styling
- Mica backdrop support
- XAML-based declarative UI
- DispatcherQueue timer for animations
- MSIX packaging support
- Cross-platform architecture support (x86, x64, ARM64)

**Key Files:**
- `App.xaml` / `App.xaml.cs` - Application entry point
- `MainWindow.xaml` / `MainWindow.xaml.cs` - Main application window
- `OverlayWindow.xaml` / `OverlayWindow.xaml.cs` - Overlay window implementation

**Build Requirements:**
- Visual Studio 2022 or later
- .NET 8 SDK
- Windows App SDK
- WinUI workload

## Common Functionality

All three implementations provide the same core functionality:

1. **Main Window**: A simple window with a button to show the overlay
2. **Overlay Window**: 
   - Displays an analog clock showing current time
   - Covers the entire screen
   - Click-through enabled (mouse events pass through to windows below)
   - Semi-transparent (50% opacity initially)
   - Automatically fades out and closes after 3 seconds

## Technical Implementation Details

### Click-Through Mechanism

All implementations use the Win32 `WS_EX_TRANSPARENT` extended window style to enable click-through behavior:

```cpp
// C++
SetWindowLong(hWnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);

// C# (WPF/WinUI via P/Invoke)
SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
```

### Transparency and Layering

- **C++**: Uses `WS_EX_LAYERED` with `SetLayeredWindowAttributes`
- **WPF**: Uses `AllowsTransparency` property and `WindowStyle.None`
- **WinUI**: Uses `WS_EX_LAYERED` with `SetLayeredWindowAttributes` via P/Invoke

### Animation

- **C++**: Custom timer-based animation using `SetTimer`
- **WPF**: XAML Storyboard with `DoubleAnimation`
- **WinUI**: `DispatcherQueueTimer` for frame-by-frame updates

## Code Reviews

Each project has been reviewed for code quality and best practices:

- [C++ Code Review](cpp/review.md)
- [WPF Code Review](wpf/review.md)
- [WinUI Code Review](WinUI/review.md)

## Building and Running

### C++ Project

1. Open Visual Studio
2. Open the `cpp` folder or project file
3. Build and run (F5)

### WPF Project

1. Open Visual Studio 2022
2. Open `wpf/wpf.csproj`
3. Restore NuGet packages
4. Build and run (F5)

### WinUI Project

1. Open Visual Studio 2022
2. Open `WinUI/WinUI.csproj`
3. Restore NuGet packages
4. Build and run (F5)

## Key Differences Between Implementations

| Feature | C++ / Win32 | WPF | WinUI 3 |
|---------|-------------|-----|---------|
| Language | C++ | C# | C# |
| .NET Version | N/A | .NET 10 | .NET 8 |
| UI Framework | Win32 | WPF | WinUI 3 |
| Graphics | GDI+ | WPF Rendering | WinUI Composition |
| XAML Support | No | Yes | Yes |
| Packaging | Traditional EXE | Traditional EXE | MSIX |
| Memory Management | Manual | Garbage Collected | Garbage Collected |
| Nullable Types | N/A | Enabled | Enabled |

## Known Issues and Improvements

Please refer to the individual code review documents for detailed analysis of issues and recommended improvements:

### Critical Issues
- Memory management concerns (especially in C++ and WinUI)
- Resource cleanup improvements needed
- Error handling enhancements required

### Recommended Improvements
- Replace magic numbers with named constants
- Add proper error handling for Win32 API calls
- Improve code organization (separate Win32 helpers)
- Remove unused code and using statements
- Add XML documentation comments

## License

This is a demonstration project. Please check individual file headers for any licensing information.

## Contributing

This project is primarily for demonstration and learning purposes. Code reviews and suggestions are documented in the respective `review.md` files for each project.

## Requirements

### Minimum System Requirements
- Windows 10, version 1809 (build 17763) or later
- For WinUI: Windows 10, version 1809 or later

### Development Requirements
- Visual Studio 2019 or later (2022 recommended)
- Windows SDK 10.0.19041.0 or later
- .NET 8 SDK (for WinUI)
- .NET 10 SDK (for WPF)

## Purpose

This repository serves as a comparison and learning resource for developers who want to understand:

1. How to implement click-through windows in different Windows frameworks
2. Differences between native Win32, WPF, and WinUI 3 development
3. Best practices for each framework
4. Trade-offs between different approaches

## Additional Resources

- [Win32 API Documentation](https://docs.microsoft.com/en-us/windows/win32/)
- [WPF Documentation](https://docs.microsoft.com/en-us/dotnet/desktop/wpf/)
- [WinUI 3 Documentation](https://docs.microsoft.com/en-us/windows/apps/winui/)
- [Windows App SDK](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/)
