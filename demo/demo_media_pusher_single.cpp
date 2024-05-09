#include "../media_pusher_single.h"
#include <iostream>
#include <string>

#if 1
int main()
{
    std::string url_src = "rtsp://192.168.1.12:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream";
    std::string url_dst = "rtsp://139.155.88.38:554/live/test";
    
    ParamMediaPusherSingle param;
    param.local_rtsp_url = url_src;
    param.rtsp_push_url = url_dst;
    MediaPusherSingle pusher;
    if (!pusher.init(param))
    {
        printf("init failed\n");
        return -1;
    }
    pusher.start();

    while (true)
    {
        std::string msg;
        std::cin >> msg;
        if (msg == "q") 
        {
            break;
        }
        if (!pusher.isConnected())
        {
            printf("pusher connect is stop\n");
            break;
        }
    }
    printf("exit...\n");
    pusher.stop();// 这里故意来验证stop函数可以被调用多次
    pusher.stop();
    pusher.stop();
    return 0;
}

#endif
