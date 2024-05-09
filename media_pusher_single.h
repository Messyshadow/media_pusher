#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <functional>

struct ParamMediaPusherSingle
{
    std::string local_rtsp_url = "";
    std::string rtsp_push_url = "";
    uint32_t framerate = 25;
    uint32_t timeout_sec = 1;
    uint32_t size_buffer_kb = 1024 * 4;
    bool is_udp = true;
};

namespace xop {
    class EventLoop;
    class RtspPusher;
};
class CameraCapture;

class MediaPusherSingle
{
public:
    MediaPusherSingle() = default;
    ~MediaPusherSingle(){close();}
    bool init(const ParamMediaPusherSingle& param_in);
    void close();
    bool start();
    void stop();
    bool isConnected();
    bool isTimeout(uint32_t timeinterval_sec);// 新增，待实现，超时判断，入参为超时阈值

private:
    void threadEncodeVideoFunc();
    bool isKeyFrame(const uint8_t* data, uint32_t size);
    void pushVideo(const uint8_t* data, uint32_t size, uint32_t timestamp);

private:
    ParamMediaPusherSingle param;
    std::shared_ptr<CameraCapture> sp_camera_capture;
    std::thread encode_video_thread;
    bool flag_init = false;
    bool flag_exit = true;
    std::mutex mutex_pusher;

    // streamer
    //xop::MediaSessionId media_session_id = 0;
    std::shared_ptr<xop::RtspPusher> sp_rtsp_pusher;
    std::shared_ptr<xop::EventLoop> sp_event_loop;

};