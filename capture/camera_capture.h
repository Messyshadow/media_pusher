#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <iostream>
#include <vector>
#include <mutex>

struct ParamCameraCapture
{
    std::string local_rtsp_url = "";
    uint32_t timeout_sec = 1;
    uint32_t size_buffer_kb = 1024 * 4;
    bool is_udp = true;
};

class CameraCapture 
{
public:
    CameraCapture();
    virtual ~CameraCapture();
    bool init(const ParamCameraCapture& param_in);
    bool close();
    bool captureFrame(std::vector<uint8_t>&image_in, uint32_t& width_in, uint32_t& height_in, int& pixel_format_in);
    uint32_t getWidth()  const ;
    uint32_t getHeight() const ;
    bool captureStarted() const ;


private:
    ParamCameraCapture param;
    AVDictionary* options = nullptr;
    AVFormatContext* format_context = nullptr;

    bool flag_init = false;
    int video_index = -1;
    std::mutex mutex_capture;
    std::shared_ptr<uint8_t> image;
    uint32_t image_size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    int pixel_format = -1;
};

#endif // CAMERACAPTURE_H
