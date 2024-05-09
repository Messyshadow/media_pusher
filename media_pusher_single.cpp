#include "media_pusher_single.h"
#include "capture/camera_capture.h"
#include "capture/common.h"
#include "net/EventLoop.h"
#include "xop/H264Parser.h"
#include "xop/RtspPusher.h"
#include "net/Timestamp.h"

bool MediaPusherSingle::init(const ParamMediaPusherSingle& param_in)
{
    if (flag_init) close();
    param = param_in;
    if (param.local_rtsp_url.empty())
    {
        LOG("local_rtsp_url is empty!\n");
        return false;
    }
    sp_event_loop = std::make_shared<xop::EventLoop>();
    flag_init = true;
    return true;
}

void MediaPusherSingle::close()
{
    stop();
    if (sp_camera_capture) sp_camera_capture->close();
    flag_init = false;
}

bool MediaPusherSingle::start()
{
    if (!flag_init)
    {
        LOG("not init succeess!\n");
        return false;
    }

    flag_exit = false;
    encode_video_thread = std::thread(std::bind(&MediaPusherSingle::threadEncodeVideoFunc, this));
    return true;
}

void MediaPusherSingle::stop()
{
    flag_exit = true;
    if (encode_video_thread.joinable()) encode_video_thread.join();
    if (sp_rtsp_pusher != nullptr)
    {
        sp_rtsp_pusher->Close();
        sp_rtsp_pusher.reset();
        LOG("RTSP Pusher stop. \n");
    }
}

bool MediaPusherSingle::isConnected()
{
    std::unique_lock<std::mutex> locker(mutex_pusher);
    bool is_connected = false;
    if (sp_rtsp_pusher != nullptr) is_connected = sp_rtsp_pusher->IsConnected();

    return is_connected;
}

bool MediaPusherSingle::isKeyFrame(const uint8_t* data, uint32_t size)
{
    if (size > 4)
    {//0x67:sps ,0x65:IDR, 0x6: SEI
        if (data[4] == 0x67 || data[4] == 0x65 ||
            data[4] == 0x6 || data[4] == 0x27)
        {
            return true;
        }
    }
    return false;
}

void MediaPusherSingle::pushVideo(const uint8_t* data, uint32_t size, uint32_t timestamp)
{
    xop::AVFrame video_frame(size);
    video_frame.size = size - 4; /* -4 ȥH.264ʼ */
    video_frame.type = isKeyFrame(data, size) ? xop::VIDEO_FRAME_I : xop::VIDEO_FRAME_P;
    video_frame.timestamp = timestamp;
    memcpy(video_frame.buffer.get(), data + 4, size - 4);

    if (size > 0)
    {
        std::unique_lock<std::mutex> locker(mutex_pusher);
        /* RTSP */
        if (sp_rtsp_pusher != nullptr && sp_rtsp_pusher->IsConnected())
        {
            sp_rtsp_pusher->PushFrame(xop::channel_0, video_frame);
        }
    }
}

void MediaPusherSingle::threadEncodeVideoFunc()
{
    xop::Timestamp  update_ts;
    uint32_t delay = 1000 / (param.framerate * 10);
    std::vector<uint8_t> bgra_image;
    int frame_size = 0;
    uint32_t width = 0, height = 0;
    int input_format = -1;
    uint32_t max_retry_count = 45;
    uint32_t retry_count = 0;
    while (!flag_exit)
    {
        if (!sp_camera_capture) sp_camera_capture = std::make_shared<CameraCapture>();
        ParamCameraCapture param_capture;
        param_capture.local_rtsp_url = param.local_rtsp_url;
        param_capture.timeout_sec = param.timeout_sec;
        param_capture.size_buffer_kb = param.size_buffer_kb;
        param_capture.is_udp = param.is_udp;
        if (!sp_camera_capture->init(param_capture))
        {
            if (flag_exit) break;
            LOG("Screen capture start failed, url: %s \n", param.local_rtsp_url.c_str());
            //sp_screen_capture.reset();
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue;
        }
        if (sp_rtsp_pusher != nullptr)
        {
            sp_rtsp_pusher->Close();
            //sp_rtsp_pusher.reset();
            LOG("RTSP Pusher close ,then reconnect.! \n");
        }
        if (!sp_rtsp_pusher) sp_rtsp_pusher = xop::RtspPusher::Create(sp_event_loop.get());
        xop::MediaSession* session = xop::MediaSession::CreateNew();
        session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
        sp_rtsp_pusher->AddSession(session);
        if (sp_rtsp_pusher->OpenUrl(param.rtsp_push_url, 1000) != 0)
        {
            LOG("RTSP Pusher: Open url(%s) failed. \n", param.rtsp_push_url.c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue;
        }
        LOG("RTSP Pusher start: Push stream to  %s ... \n", param.rtsp_push_url.c_str());
        retry_count = 0;
        while (!flag_exit)
        {
            uint32_t timestamp = xop::H264Source::GetTimestamp();
            if (sp_camera_capture->captureFrame(bgra_image, width, height, input_format))
            {
                if (input_format == AVMEDIA_TYPE_VIDEO)
                {
                    if (bgra_image.size() > 0)
                    {
                        pushVideo(&bgra_image[0], bgra_image.size(), timestamp);
                    }
                }
            }
            else
            {
                if (retry_count < max_retry_count)
                {
                    retry_count++;
                    continue;
                }
                else
                {
                    LOG("VideoCapMgrRtsp continuous lost, reinit,local rtsp url : %s, remote rtsp url : %s\n ", param.local_rtsp_url.c_str(), param.rtsp_push_url.c_str());
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }
}


