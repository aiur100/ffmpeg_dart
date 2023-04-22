import 'package:dart_ffmpeg_lib/dart_ffmpeg_lib.dart' as dart_ffmpeg_lib;
import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:path/path.dart' as path;

typedef GetFFmpegVersionC = Pointer<Utf8> Function();
typedef GetFFmpegVersion = Pointer<Utf8> Function();
typedef ConcatenateVideosC = Int32 Function(
    Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);
typedef ConcatenateVideos = int Function(
    Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);

void main(List<String> arguments) {
  DynamicLibrary dylib = ffmpegWrapperCCode();
  final version = getVersion(dylib);
  print('FFmpeg version: $version');
  String currentDir = Directory.current.path;
  String videoOne = path.join(currentDir, 'test/test_video_1.mp4');
  String videoTwo = path.join(currentDir, 'test/test_video_2.mp4');
  String output = path.join(currentDir, 'test/combined.mp4');

  print('Combining $videoOne and $videoTwo to $output');
  if (concatenateTwoVideos(dylib, videoOne, videoTwo, output)) {
    print('Videos combined at $output');
  } else {
    print('Some error occurred!');
  }
}

String getVersion(DynamicLibrary dylib) {
  final getFFmpegVersionPointer =
      dylib.lookup<NativeFunction<GetFFmpegVersionC>>('get_ffmpeg_version');
  final getFFmpegVersion =
      getFFmpegVersionPointer.asFunction<GetFFmpegVersion>();
  return getFFmpegVersion().toDartString();
}

bool concatenateTwoVideos(DynamicLibrary dylib, String firstVideoPath,
    String secondVideoPath, String outputPath) {
  // Add this inside the main function, after loading the dynamic library
  final concatenateMediaPointer =
      dylib.lookup<NativeFunction<ConcatenateVideosC>>('concatenate_videos');
  final concatenateMedia =
      concatenateMediaPointer.asFunction<ConcatenateVideos>();

  final input1Ptr = firstVideoPath.toNativeUtf8();
  final input2Ptr = secondVideoPath.toNativeUtf8();
  final outputPtr = outputPath.toNativeUtf8();
  int result = concatenateMedia(input1Ptr, input2Ptr, outputPtr);
  return result == 0;
}

/// Set-up/start-up function.
/// ---------------------------
/// Loads compiled C code and returns
/// the API as a dynamic library.
DynamicLibrary ffmpegWrapperCCode() {
  String currentDir = Directory.current.path;
  String libPath = path.join(currentDir, 'ffmpeg_wrapper.so');
  return DynamicLibrary.open(libPath);
}
