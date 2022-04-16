import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

class KeySimulator {
  static const MethodChannel _channel = MethodChannel('key_simulator');

  static const _eventChannel =
      EventChannel('key_simulator/clipboard_event_callback');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Stream<String> clipboardEventStream() {
    return _eventChannel.receiveBroadcastStream().map((message) {
      return message as String;
    });
  }

  static Future<bool> get simulateCopy async {
    return await _channel.invokeMethod('simulateCopy');
  }

  static Future<bool> get simulateTabShifter async {
    return await _channel.invokeMethod('simulateTabShifter');
  }
}
