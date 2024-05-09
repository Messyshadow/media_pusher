#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <mutex>

struct ParamMediaSourceInfo
{
    void debug() const
    {
        std::cout << "main_rtsp :" << main_rtsp << std::endl;
        std::cout << "sub_rtsp :" << sub_rtsp << std::endl;
        std::cout << "manufacturer :" << manufacturer << std::endl;
        std::cout << "model :" << model << std::endl;
        std::cout << "serial_number :" << serial_number << std::endl;
        std::cout << "hardwareid :" << hardwareid << std::endl;
        std::cout << "firmware_version :" << firmware_version << std::endl;
    }
    bool operator== (const ParamMediaSourceInfo& other) const
    {
        return main_rtsp == other.main_rtsp && sub_rtsp == other.sub_rtsp &&
            manufacturer == other.manufacturer && model == other.model &&
            serial_number == other.serial_number && hardwareid == other.hardwareid &&
            firmware_version == other.firmware_version;
    }
    std::string main_rtsp; //主码流
    std::string sub_rtsp; //辅码流
    std::string manufacturer;//制造商
    std::string model;//模型
    std::string serial_number;//序列号
    std::string hardwareid;//硬件id
    std::string firmware_version;//固件版本
};

enum EnumUdpFindMethod
{
    broadcast = 0, //广播
    multicast,//多播
};

struct ParamMediaFinderOnvif
{
    uint32_t timeout_sec = 10;
    std::string username = "admin";
    std::string password = "";
    EnumUdpFindMethod udp_find_method = EnumUdpFindMethod::broadcast;
    std::string broadcast_url = "192.168.1.255";
};

class MediaFinderOnvif
{
public:
    bool init(const ParamMediaFinderOnvif& param_in);
    bool findMedia(std::vector<ParamMediaSourceInfo>& vec_media);

private:
    bool serachMediaSourceInfo(std::vector<ParamMediaSourceInfo>& vec_media, EnumUdpFindMethod udp_find_method, 
         const std::string& ipv4_address = "");

    bool getDeviceInformation(const char* DeviceXAddr, std::string& manufacturer, std::string& model,
                              std::string& serial_number, std::string& hardwareid, std::string& firmware_version);
    bool getOnvifRtsp(const char* device,
        char* main_rtsp, char* sub_rtsp,
        const char* user, const char* pass);

private:
    ParamMediaFinderOnvif param;
    std::mutex mtx;
};


