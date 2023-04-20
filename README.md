# FFMPEG AND DART

Authors: 
- Christopher Rene Hill (@aiur100) chrishill9@gmail.com
- Nick Ricciuti (@rishooty) rishooty@gmail.com

### Compile FFMPEG First.
*Requires that you have ffmpeg sources installed.*

Run this command to compile FFMPEG. 
```
gcc -I /opt/homebrew/Cellar/ffmpeg/6.0/include -shared -o ffmpeg_wrapper.so ffmpeg_wrapper.c -L /opt/homebrew/Cellar/ffmpeg/6.0/lib -lavutil -lavcodec -lavformat -fPIC
```
Replace the -I with the `/include` directory of your installation of FFMPEG
Replace the -L with your `/lib` directory of your installation of FFMPEG

## Running the program 
```
dart run
```

## What's happening?

This code demonstrates how to load a shared library (in this case, ffmpeg_wrapper.so) and call a function (get_ffmpeg_version) from that library using Dart FFI (Foreign Function Interface). 
The C code we are importing is in `ffmpeg_wrapper.c`. 







