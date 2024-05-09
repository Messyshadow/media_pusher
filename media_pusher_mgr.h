#pragma once
#include <map>
#include "media_finder.h"
#include "media_pusher_single.h"

struct ParamMediaPusherMulti
{
};

class MediaPusherMgr
{
public:
    MediaPusherMgr() = default;
    ~MediaPusherMgr(){}
    bool init(
        const ParamMediaPusherMulti& param_in,
        const std::shared_ptr<MeidaFinderBasis>& sp_finder_in);

    // 新增一个推流源，入参为指针，方便以后多态
    bool add(const ParamMediaSourceBasis* p_param_source);
    
    // 移除一个推流源，入参为指针，方便以后多态
    // 注，并不是一发现有设备掉线，就把这个pusher移除，而应该有超时判断，比如掉线一周才移除等
    bool remove(const ParamMediaSourceBasis* p_param_source);


private:
    ParamMediaPusherMulti param;
    std::shared_ptr<MeidaFinderBasis> sp_finder;
    std::map<std::string, std::shared_ptr<MediaPusherSingle>> lut_pusher;
};