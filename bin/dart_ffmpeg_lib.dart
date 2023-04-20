import 'package:dart_ffmpeg_lib/dart_ffmpeg_lib.dart' as dart_ffmpeg_lib;
import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:path/path.dart' as path;

typedef GetFFmpegVersionC = Pointer<Utf8> Function();
typedef GetFFmpegVersion = Pointer<Utf8> Function();

void main(List<String> arguments) {
  String currentDir = Directory.current.path;
  String libPath = path.join(currentDir, 'ffmpeg_wrapper.so');
  final dylib = DynamicLibrary.open(libPath);

  final getFFmpegVersionPointer =
      dylib.lookup<NativeFunction<GetFFmpegVersionC>>('get_ffmpeg_version');
  final getFFmpegVersion =
      getFFmpegVersionPointer.asFunction<GetFFmpegVersion>();

  final version = getFFmpegVersion().toDartString();
  print('FFmpeg version: $version');
}
