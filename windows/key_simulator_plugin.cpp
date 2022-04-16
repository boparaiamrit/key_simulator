#include "include/key_simulator/key_simulator_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <flutter/event_channel.h>
#include <mutex>

namespace
{
  template <typename T = flutter::EncodableValue>
  class ClipboardStreamHandler : public flutter::StreamHandler<T>
  {
  public:
    ClipboardStreamHandler() = default;
    virtual ~ClipboardStreamHandler() = default;

    void onCallback(flutter::EncodableValue _data)
    {
      std::unique_lock<std::mutex> _ul(mMtx);
      if (mSink.get())
        mSink.get()->Success(_data);
    }

  protected:
    std::unique_ptr<flutter::StreamHandlerError<T>> OnListenInternal(const T *arguments, std::unique_ptr<flutter::EventSink<T>> &&events) override
    {
      std::unique_lock<std::mutex> _ul(mMtx);
      mSink = std::move(events);
      return nullptr;
    }
    std::unique_ptr<flutter::StreamHandlerError<T>> OnCancelInternal(const T *arguments) override
    {
      std::unique_lock<std::mutex> _ul(mMtx);
      mSink.release();
      return nullptr;
    }

  private:
    flutter::EncodableValue mValue;
    std::mutex mMtx;
    std::unique_ptr<flutter::EventSink<T>> mSink;
  };

  class KeySimulatorPlugin : public flutter::Plugin
  {
  public:
    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    KeySimulatorPlugin(flutter::PluginRegistrarWindows *registrar);

    virtual ~KeySimulatorPlugin();

  private:
    int windowProcId = -1;
    std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> clipbaordEventChannel;
    ClipboardStreamHandler<> *clipboardHandler;
    flutter::PluginRegistrarWindows *registrar;

    // Called for top-level WindowProc delegation.
    std::optional<LRESULT> KeySimulatorPlugin::HandleWindowProc(HWND hWnd,
                                                                UINT message,
                                                                WPARAM wParam,
                                                                LPARAM lParam);

    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  };

  // static
  void KeySimulatorPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "key_simulator",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<KeySimulatorPlugin>(registrar);

    plugin->clipbaordEventChannel = std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
        registrar->messenger(), "key_simulator/clipboard_event_callback",
        &flutter::StandardMethodCodec::GetInstance());

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    ClipboardStreamHandler<> *_handler = new ClipboardStreamHandler<>();
    plugin->clipboardHandler = _handler;
    auto _obj_stm_handle = static_cast<flutter::StreamHandler<flutter::EncodableValue> *>(plugin->clipboardHandler);
    std::unique_ptr<flutter::StreamHandler<flutter::EncodableValue>> _ptr{_obj_stm_handle};
    plugin->clipbaordEventChannel->SetStreamHandler(std::move(_ptr));

    registrar->AddPlugin(std::move(plugin));
  }

  std::optional<LRESULT> KeySimulatorPlugin::HandleWindowProc(HWND hWnd,
                                                              UINT message,
                                                              WPARAM wParam,
                                                              LPARAM lParam)
  {
    std::optional<LRESULT> result = std::nullopt;

    switch (message)
    {
    case WM_CLIPBOARDUPDATE:
      if (clipboardHandler != nullptr)
      {
        clipboardHandler->onCallback(flutter::EncodableValue("CLIPBOARD_UPDATED"));
      }

      return 0;
    }

    return result;
  }

  KeySimulatorPlugin::KeySimulatorPlugin(flutter::PluginRegistrarWindows *registrar)
  {
    this->registrar = registrar;
    windowProcId = registrar->RegisterTopLevelWindowProcDelegate(
        [this](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
          return HandleWindowProc(hWnd, message, wParam, lParam);
        });
  }

  KeySimulatorPlugin::~KeySimulatorPlugin()
  {
    registrar->UnregisterTopLevelWindowProcDelegate(windowProcId);
  }

  void KeySimulatorPlugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    if (method_call.method_name().compare("getPlatformVersion") == 0)
    {
      std::ostringstream version_stream;
      version_stream << "Windows ";
      if (IsWindows10OrGreater())
      {
        version_stream << "10+";
      }
      else if (IsWindows8OrGreater())
      {
        version_stream << "8";
      }
      else if (IsWindows7OrGreater())
      {
        version_stream << "7";
      }
      result->Success(flutter::EncodableValue(version_stream.str()));
    }
    else
    {
      if (method_call.method_name().compare("simulateCopy") == 0)
      {
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Press the "Ctrl" key
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = 0; // 0 for key press
        SendInput(1, &ip, sizeof(INPUT));

        // Press the "V" key
        ip.ki.wVk = 0x56;
        ip.ki.dwFlags = 0; // 0 for key press
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "V" key
        ip.ki.wVk = 0x56;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "Ctrl" key
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        result->Success(flutter::EncodableValue(true));
      }
      else
      {
        if (method_call.method_name().compare("simulateTabShifter") == 0)
        {
          INPUT ip;
          ip.type = INPUT_KEYBOARD;
          ip.ki.wScan = 0;
          ip.ki.time = 0;
          ip.ki.dwExtraInfo = 0;

          // Press the "Ctrl" key
          ip.ki.wVk = VK_CONTROL;
          ip.ki.dwFlags = 0; // 0 for key press
          SendInput(1, &ip, sizeof(INPUT));

          // Press the "TAB" key
          ip.ki.wVk = VK_TAB;
          ip.ki.dwFlags = 0; // 0 for key press
          SendInput(1, &ip, sizeof(INPUT));

          // Release the "TAB" key
          ip.ki.wVk = VK_TAB;
          ip.ki.dwFlags = KEYEVENTF_KEYUP;
          SendInput(1, &ip, sizeof(INPUT));

          // Release the "Ctrl" key
          ip.ki.wVk = VK_CONTROL;
          ip.ki.dwFlags = KEYEVENTF_KEYUP;
          SendInput(1, &ip, sizeof(INPUT));

          result->Success(flutter::EncodableValue(true));
        }
        else
        {
          result->NotImplemented();
        }
      }
    }
  }

} // namespace

void KeySimulatorPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
  KeySimulatorPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
