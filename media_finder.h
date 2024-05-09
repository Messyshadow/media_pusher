#pragma once
#include <string>
#include <thread>
#include <functional>
#include <vector>
#include <iostream>

struct ParamMediaSourceBasis
{
    std::string url;
};

struct ParamMediaSource
{
    std::string url;
};

struct ParamMediaFinder
{
    uint32_t timeinterval_sec = 8000;
};

class MeidaFinderBasis
{
public:
    MeidaFinderBasis() = default;
    virtual ~MeidaFinderBasis() = 0;
    bool init(
        const ParamMediaFinder& param_in,
        const std::function<bool(const ParamMediaSourceBasis*)>& callback_new_in,// 回调函数，发现有新增的视频源调此函数
        const std::function<bool(const ParamMediaSourceBasis*)>& callback_miss_in// 回调函数，发现有消失的视频源调此函数
    )
    {
        param = param_in;
        flag_exit = false;
        thread = std::thread(std::bind(&MeidaFinderBasis::threadaFunc, this));
    }

    // 通过这个函数找到当前局域网内所有视频源
    virtual bool findMedia(std::vector<ParamMediaSource>& vec_finds) = 0;

protected:
    //根据两次检索结果得到视频源的变化情况
    void getMediaChange(
        const std::vector<ParamMediaSource>& vec_old,/* 上次检测到的视频源 */
        const std::vector<ParamMediaSource>& vec_curr,/* 本次检测到的视频源 */
        std::vector<ParamMediaSource>& vec_new,/* 哪些视频源是新增的 */
        std::vector<ParamMediaSource>& vec_miss);/* 哪些视频源是消失了 */

    void threadaFunc()
    {
        while (!flag_exit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(param.timeinterval_sec));
        }
    }

protected:
    ParamMediaFinder param;
    std::function<bool(const ParamMediaSourceBasis*)> callback_new;
    std::function<bool(const ParamMediaSourceBasis*)> callback_miss;
    bool flag_exit;
    std::thread thread;
};