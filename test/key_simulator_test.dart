import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:key_simulator/key_simulator.dart';

void main() {
  const MethodChannel channel = MethodChannel('key_simulator');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await KeySimulator.platformVersion, '42');
  });
}
