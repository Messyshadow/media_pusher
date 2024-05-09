#include "camera_capture.h"
#include <string>

CameraCapture::CameraCapture()
{
    avdevice_register_all();
}


CameraCapture::~CameraCapture()
{

}


bool CameraCapture::init(const ParamCameraCapture& param_in)
{
    param = param_in;
    if (param.local_rtsp_url.empty())
    {
        std::cout << "local_rtsp_url is empry!" << std::endl;
        return false;
    }
    if (flag_init) close();
    const char* protocol = param.is_udp ? "udp" : "tcp";
    std::string timeout_us_str = std::to_string(param.timeout_sec * 1000000);
    std::string size_buffer_bytes_str = std::to_string(param.size_buffer_kb * 1024);
    //AVDictionary* options = nullptr;
    av_dict_set(&options, "stimeout", timeout_us_str.c_str(), 0);
    //解决花屏和卡顿 可以设置buffer_size
    av_dict_set(&options, "buffer_size", size_buffer_bytes_str.c_str(), 0);
    av_dict_set(&options, "rtsp_transport", protocol, 0);
    avformat_network_init();
    format_context = avformat_alloc_context();
    if (avformat_open_input(&format_context, param.local_rtsp_url.c_str(), nullptr, &options) != 0)
    {
        printf("[GDIScreenCapture] Open input failed.");
        close();
        return false;
    }

    if (avformat_find_stream_info(format_context, nullptr) < 0)
    {
        printf("[GDIScreenCapture] Couldn't find stream info.\n");
        close();
        return false;
    }
    for (int i = 0; i < format_context->nb_streams; i++)
    {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
    }
    if (video_index < 0) {
        printf("[GDIScreenCapture] Couldn't find video stream.\n");
        close();
        return false;
    }
    flag_init = true;
    return true;
}

bool CameraCapture::close()
{
    if (options != nullptr)
    {
        av_dict_free(&options);
        options = nullptr;
    }
    if (format_context)
    {
        avformat_close_input(&format_context);
        format_context = nullptr;
    }
    flag_init = false;
    return true;
}
bool CameraCapture::captureFrame(std::vector<uint8_t>&image_in, uint32_t& width_in,
                                 uint32_t& height_in, int& input_format_in)
{
    image_in.clear();
    if (!flag_init)
    {
        printf("not is_initialized_\n");
        return false;
    }
    std::shared_ptr<AVPacket> av_packet(av_packet_alloc(), [](AVPacket* ptr) {av_packet_free(&ptr); });
    av_init_packet(av_packet.get());

    int ret = av_read_frame(format_context, av_packet.get());
    if (ret < 0)
    {
        av_packet_unref(av_packet.get());
        return false;
    }

    if (av_packet->stream_index == video_index) {
        if (av_packet->size <= 0)
        {
            av_packet_unref(av_packet.get());
            //printf("av_packet size is 0!\n");
            return false;
        }
        //image_in.clear();
        image_in.resize(av_packet->size);
        image_in.assign(av_packet->data, av_packet->data + av_packet->size);
        width_in = width;
        height_in = height;
        input_format_in = video_index;
    }
    else
    {
        input_format_in = AVMEDIA_TYPE_AUDIO;
    }
    av_packet_unref(av_packet.get());
    return true;
}

uint32_t CameraCapture::getWidth() const
{
    return width;
}

uint32_t CameraCapture::getHeight() const
{
    return height;
}

