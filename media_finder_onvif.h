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
    std::string main_rtsp; //������
    std::string sub_rtsp; //������
    std::string manufacturer;//������
    std::string model;//ģ��
    std::string serial_number;//���к�
    std::string hardwareid;//Ӳ��id
    std::string firmware_version;//�̼��汾
};

enum EnumUdpFindMethod
{
    broadcast = 0, //�㲥
    multicast,//�ಥ
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


