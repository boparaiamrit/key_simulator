import Cocoa
import FlutterMacOS

public class KeySimulatorPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "key_simulator", binaryMessenger: registrar.messenger)
    let instance = KeySimulatorPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "getPlatformVersion":
      result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
    case "simulateCopy":
      let event3 = CGEvent(keyboardEventSource: nil, virtualKey: 0x09, keyDown: true);
      event3?.flags = CGEventFlags.maskCommand;
      event3?.post(tap: CGEventTapLocation.cghidEventTap);

      let event4 = CGEvent(keyboardEventSource: nil, virtualKey: 0x09, keyDown: false);
      event4?.flags = CGEventFlags.maskCommand;
      event4?.post(tap: CGEventTapLocation.cghidEventTap);
      result(true)
    case "simulateTabShifter":
      let event1 = CGEvent(keyboardEventSource: nil, virtualKey: 0x30, keyDown: true);
      event1?.flags = CGEventFlags.maskCommand;
      event1?.post(tap: CGEventTapLocation.cghidEventTap);

      let event2 = CGEvent(keyboardEventSource: nil, virtualKey: 0x30, keyDown: false);
      event2?.flags = CGEventFlags.maskCommand;
      event2?.post(tap: CGEventTapLocation.cghidEventTap);
      result(true)
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}
