#ifndef FFMPEG_COMMON_H
#define FFMPEG_COMMON_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/error.h"
}
#include <cstdio>
#include <memory>
#include <string>
#ifdef _WIN32
#include <WinSock2.h>
#include <Iphlpapi.h>
#endif


namespace ffmpeg {

	using AVPacketPtr = std::shared_ptr<AVPacket>;
	using AVFramePtr = std::shared_ptr<AVFrame>;

}

struct  NetWorkInfo
{
    void debug() const
    {
        std::cout << "NetWorkInfo start debug:" << std::endl;
        std::cout << "�������ƣ�" << adapter_name << std::endl;
        std::cout << "����������" << description << std::endl;
        std::cout << "����mac��ַ��" << mac_address << std::endl;
        std::cout << "Ĭ�����أ�" << gateway_address << std::endl;
        std::cout << "����ipv4��ַ��" << local_ip_address << std::endl;
        std::cout << "�������룺" << mask_address << std::endl;
        std::cout << "NetWorkInfo end debug!" << std::endl;
    }
	std::string adapter_name;//��������
	std::string description;//��������
	std::string mac_address;//����mac��ַ
	std::string gateway_address;//Ĭ�����ص�ַ
	std::string local_ip_address;//����ip��ַ 
	std::string mask_address;//��������
};

void getLocalAllNetWorkInfo(std::vector<NetWorkInfo>& vec_network_info)
{
#ifdef _WIN32
    PIP_ADAPTER_INFO pip_adapter_info = new IP_ADAPTER_INFO();
    unsigned long size = sizeof(IP_ADAPTER_INFO);
    ::GetAdaptersInfo(pip_adapter_info, &size);
    pip_adapter_info = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, size);

    //ȡ���������Ľṹ��Ϣ
    if (::GetAdaptersInfo(pip_adapter_info, &size) == ERROR_SUCCESS)
    {
        while (pip_adapter_info)
        {
            u_char uc_local_mac[6];
            char mac_address[100] = { 0 };
            memcpy(uc_local_mac, pip_adapter_info->Address, 6);
            sprintf(mac_address, "%02X-%02X-%02X-%02X-%02X-%02X", uc_local_mac[0], uc_local_mac[1], uc_local_mac[2], 
                uc_local_mac[3], uc_local_mac[4], uc_local_mac[5]);
            NetWorkInfo network_info = { pip_adapter_info->AdapterName, pip_adapter_info->Description, mac_address,
            pip_adapter_info->GatewayList.IpAddress.String, pip_adapter_info->IpAddressList.IpAddress.String, pip_adapter_info->IpAddressList.IpMask.String };
            pip_adapter_info = pip_adapter_info->Next;
            vec_network_info.emplace_back(network_info);
        }
    }
    if (pip_adapter_info)
    {
        delete pip_adapter_info;
    }
#endif
}

#define LOG(format, ...)  	\
{								\
    fprintf(stderr, "[%s:%d] " format " \n", \
   __FUNCTION__ , __LINE__, ##__VA_ARGS__);     \
}

#define AV_LOG(code, format, ...)  	\
{								\
	char buf[1024] = { 0 };		\
	av_strerror(code, buf, 1023); \
    fprintf(stderr, "[%s:%d] " format " - %s. \n", \
   __FUNCTION__ , __LINE__, ##__VA_ARGS__, buf);     \
}

#endif
