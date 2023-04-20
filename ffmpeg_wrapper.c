#include <libavutil/avutil.h>

const char* get_ffmpeg_version() {
  return av_version_info();
}
