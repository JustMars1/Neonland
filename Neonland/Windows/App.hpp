#pragma once

#include "DeviceResources.hpp"
#include "NeonMain.hpp"

class App : public winrt::implements<App, winrt::Windows::ApplicationModel::Core::IFrameworkViewSource, winrt::Windows::ApplicationModel::Core::IFrameworkView>
{
public:
    App();
    winrt::Windows::ApplicationModel::Core::IFrameworkView CreateView();

    void Initialize(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& applicationView);
    void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);
    void Load(winrt::hstring const&);
    void Run();
    void Uninitialize();
protected:
    void OnActivated(winrt::Windows::ApplicationModel::Core::CoreApplicationView const&, winrt::Windows::ApplicationModel::Activation::IActivatedEventArgs const&);

    void OnWindowSizeChanged(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::WindowSizeChangedEventArgs const& args);
    void OnVisibilityChanged(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args);
    void OnWindowClosed(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::CoreWindowEventArgs const&);

    void OnDpiChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, IInspectable const&);
    void OnOrientationChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, IInspectable const&);
    void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const&, IInspectable const&);

    void OnPointerPressed(IInspectable const&, winrt::Windows::UI::Core::PointerEventArgs const& args);
    void OnPointerReleased(IInspectable const&, winrt::Windows::UI::Core::PointerEventArgs const& args);
    void OnPointerMoved(IInspectable const&, winrt::Windows::UI::Core::PointerEventArgs const& args);

    void OnKeyDown(IInspectable const&, winrt::Windows::UI::Core::KeyEventArgs const&);
    void OnKeyUp(IInspectable const&, winrt::Windows::UI::Core::KeyEventArgs const&);

    void OnInputDirChanged();
private:
    std::shared_ptr<DeviceResources> GetDeviceResources();

    std::shared_ptr<DeviceResources> _deviceResources;
    std::unique_ptr<NeonMain> _main;

    bool _windowClosed;
    bool _windowVisible;

    bool forwardHeld;
    bool backHeld;
    bool rightHeld;
    bool leftHeld;
};