#include "media_finder_onvif.h"
#include "onvif/soapH.h"
#include "onvif/wsseapi.h"
#include "onvif/wsdd.nsmap"
#include "onvif/wsaapi.h"
#include <thread>

#include "capture/common.h"
#include <assert.h>
#include <regex>

#define MULTICAST_ADDRESS "soap.udp://239.255.255.250:3702"  
#define BROADCAST_ADDRESS "soap.udp://192.168.1.255:3702"  

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_TYPES  "dn:NetworkVideoTransmitter"

static bool checkIPAddrIsVaild(const std::string& str)
{
	std::regex reg("(?=(\\b|\\D))(((\\d{1,2})|(1\\d{1,2})|(2[0-4]\\d)|(25[0-5]))\\.){3}((\\d{1,2})|(1\\d{1,2})|(2[0-4]\\d)|(25[0-5]))(?=(\\b|\\D))");
	return std::regex_match(str, reg);
}

static std::vector<std::string> getAllLocalEthernet2Broadvast()
{
	
	std::vector<std::string> vec_local_ethernetnet;
	std::string broadcast_onvif_url = "soap.udp://192.168.1.255:3702";
	vec_local_ethernetnet.push_back(broadcast_onvif_url);
	return vec_local_ethernetnet;
}

static struct soap* soapNew(int timeout_sec)
{
	LOG("soap start new !\n");
	/// 初始化soap
	struct soap* temp_soap = NULL;
	temp_soap = soap_new();
	//设置命名空间
	soap_set_namespaces(temp_soap, namespaces);
	//设置超时时间
	temp_soap->recv_timeout = timeout_sec;
	temp_soap->send_timeout = timeout_sec;
	temp_soap->connect_timeout = timeout_sec;
	//设置xml soap消息编码格式 utf
	soap_set_mode(temp_soap, SOAP_C_UTFSTRING);
	return temp_soap;
}

static void soapFree(soap* soap)
{
	soap_destroy(soap);
	soap_end(soap);
	soap_done(soap);
	soap_free(soap);
	soap = nullptr;
	LOG("soap free done!\n");
}

void soapPerror(struct soap* soap, const char* str)
{
	if (NULL == str) {
		printf("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	else {
		printf("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	return;
}

#define SOAP_CHECK_ERROR(result, soap, str) \
    if (SOAP_OK != (result) || SOAP_OK != (soap)->error) { \
        soapPerror((soap), (str)); \
        if (SOAP_OK == (result)) { \
            (result) = (soap)->error; \
        } \
		return false; \
    } 

static int setAuthInfo(struct soap* soap, const char* username, const char* password)
{
	int result = 0;
	assert(NULL != username);
	assert(NULL != password);
	result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
	SOAP_CHECK_ERROR(result, soap, "add_UsernameTokenDigest");
	return result;
}



bool MediaFinderOnvif::init(const ParamMediaFinderOnvif& param_in)
{
	param = param_in;
	return true;
}

bool MediaFinderOnvif::findMedia(std::vector<ParamMediaSourceInfo>& vec_media)
{
	std::unique_lock<std::mutex>lk(mtx);
	std::vector<NetWorkInfo> vec_network_info;
	getLocalAllNetWorkInfo(vec_network_info);
	printf("findMedia start!\n");
	vec_media.clear();
	if (param.udp_find_method == EnumUdpFindMethod::broadcast)
	{
		bool flag_searchmedia = false;
		for (const auto& item : vec_network_info)
		{
			if (!checkIPAddrIsVaild(item.local_ip_address) || item.local_ip_address == "0.0.0.0")
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			}
			std::cout << "local ip address = " << item.local_ip_address << std::endl;
			if (!serachMediaSourceInfo(vec_media, EnumUdpFindMethod::broadcast, item.local_ip_address))
			{
				LOG("local ip : %s search media info failed!\n", item.local_ip_address.c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
			flag_searchmedia = true;
		}
		if (!flag_searchmedia)
		{
			LOG("serachMediaSourceInfo all broadcast is failed!\n");
			return false;
		}
	}
	else if (param.udp_find_method == EnumUdpFindMethod::multicast)
	{
		if (!serachMediaSourceInfo(vec_media, EnumUdpFindMethod::multicast))
		{
			LOG("serachMediaSourceInfo multicast is failed!\n");
			return false;
		}
	}
	return true;
}

bool MediaFinderOnvif::serachMediaSourceInfo(std::vector<ParamMediaSourceInfo>& vec_media, EnumUdpFindMethod udp_find_method, 
	const std::string& ipv4_address)
{
	if (ipv4_address == "0.0.0.0") return false;
	struct soap* p_soap = nullptr;
	p_soap = soapNew(param.timeout_sec);
	if (!p_soap)
	{
		LOG("p_soap is nullptr\n");
		return false;
	}

	printf("start search device!...\n");
	int result = 0;
	char cams[128][1024] = { 0 };
	wsdd__ProbeType req;
	struct __wsdd__ProbeMatches resp;
	wsdd__ScopesType scope_type;
	struct SOAP_ENV__Header header;
	soap_default_SOAP_ENV__Header(p_soap, &header);
	char guid_string[100];
	header.wsa__MessageID = guid_string;
	header.wsa__MessageID = (char*)soap_wsa_rand_uuid(p_soap);
	header.wsa__To = (char*)soap_malloc(p_soap, strlen(SOAP_TO) + 1);
	header.wsa__Action = (char*)soap_malloc(p_soap, strlen(SOAP_ACTION) + 1);
	strcpy(header.wsa__To, SOAP_TO);
	strcpy(header.wsa__Action, SOAP_ACTION);
	//header.wsa__To = (char*)SOAP_TO;
	//header.wsa__Action = (char*)SOAP_ACTION;
	p_soap->header = &header;

	soap_default_wsdd__ScopesType(p_soap, &scope_type);
	scope_type.__item = (char*)"";
	soap_default_wsdd__ProbeType(p_soap, &req);
	req.Scopes = &scope_type;
	req.Types = (char*)soap_malloc(p_soap, strlen(SOAP_TYPES) + 1);
	strcpy(req.Types, SOAP_TYPES);
	int count = 0;
	int i = 0;
	if (udp_find_method == EnumUdpFindMethod::broadcast)
	{
		std::string broadcast_url;
		broadcast_url = ipv4_address.substr(0, ipv4_address.rfind("."));
		broadcast_url = "soap.udp://" + broadcast_url;
		broadcast_url.append(".255:3702");
		std::cout << "broadcast_url = " << broadcast_url << std::endl;
		result = soap_send___wsdd__Probe(p_soap, broadcast_url.c_str()/*BROADCAST_ADDRESS*/, NULL, &req);
	}
	else if(udp_find_method == EnumUdpFindMethod::multicast)
	{
		result = soap_send___wsdd__Probe(p_soap, MULTICAST_ADDRESS, NULL, &req);
	}

	while (result == SOAP_OK)
	{
		memset(&resp, 0x00, sizeof(resp));
		result = soap_recv___wsdd__ProbeMatches(p_soap, &resp);
		if (SOAP_OK == result)
		{
			if (p_soap->error)
			{
				printf("soap error 1: %d, %s, %s\n", p_soap->error, *soap_faultcode(p_soap), *soap_faultstring(p_soap));
				result = p_soap->error;
			}
			else
			{
				printf("guog *********************************************\r\n");
				if (p_soap->header->wsa__MessageID)
				{
					printf("MessageID   : %s\r\n", p_soap->header->wsa__MessageID);
				}
				if (p_soap->header->wsa__RelatesTo && p_soap->header->wsa__RelatesTo->__item)
				{
					printf("RelatesTo   : %s\r\n", p_soap->header->wsa__RelatesTo->__item);
				}
				if (p_soap->header->wsa__To)
				{
					printf("To          : %s\r\n", p_soap->header->wsa__To);
				}
				if (p_soap->header->wsa__Action)
				{
					printf("Action      : %s\r\n", p_soap->header->wsa__Action);
				}
				for (i = 0; i < resp.wsdd__ProbeMatches->__sizeProbeMatch; i++)
				{
					printf("__sizeProbeMatch        : %d\r\n", resp.wsdd__ProbeMatches->__sizeProbeMatch);
					printf("wsa__EndpointReference       : %p\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference);
					printf("Target EP Address       : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
					printf("Target Type             : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Types);
					printf("Target Service Address  : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
					printf("Target Metadata Version : %d\r\n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
					if (resp.wsdd__ProbeMatches->ProbeMatch->Scopes)
					{
						printf("Target Scopes Address   : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
					}
					strcpy(cams[count], resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
					count++;
				}
			}
		}
		else if (p_soap->error)
		{
			printf("[%d] soap error 2: %d, %s, %s\n", __LINE__, p_soap->error, *soap_faultcode(p_soap), *soap_faultstring(p_soap));
			result = p_soap->error;
		}
	}
	soapFree(p_soap);
	if (count <= 0) return false;
	printf("the device count : %d\n", count);
	for (int i = 0; i < count; i++)
	{
		ParamMediaSourceInfo param_media_source_info;
		getDeviceInformation(cams[i], param_media_source_info.manufacturer, param_media_source_info.model,
			param_media_source_info.serial_number, param_media_source_info.hardwareid, param_media_source_info.firmware_version);
		char main_rtsp[1024] = { 0 };
		char sub_rtsp[1024] = { 0 };
		getOnvifRtsp(cams[i], main_rtsp, sub_rtsp,
			param.username.c_str(), param.password.c_str());
		param_media_source_info.main_rtsp = main_rtsp;
		param_media_source_info.sub_rtsp = sub_rtsp;
		vec_media.push_back(param_media_source_info);
	}
	return true;
}

bool MediaFinderOnvif::getDeviceInformation(const char* DeviceXAddr, std::string& manufacturer, std::string& model,
	std::string& serial_number, std::string& hardwareid, std::string& firmware_version)
{
	int result = 0;
	struct soap* soap = NULL;
	struct _tds__GetDeviceInformation           devinfo_req;
	struct _tds__GetDeviceInformationResponse   devinfo_resp;
	assert(DeviceXAddr != NULL);
	soap = soapNew(param.timeout_sec);
	setAuthInfo(soap, param.username.c_str(), param.password.c_str());
	memset(&devinfo_req, 0x00, sizeof(devinfo_req));
	memset(&devinfo_resp, 0x00, sizeof(devinfo_resp));
	result = soap_call___tds__GetDeviceInformation(soap, DeviceXAddr, NULL, &devinfo_req, &devinfo_resp);
	SOAP_CHECK_ERROR(result, soap, "GetDeviceInformation");
	manufacturer = devinfo_resp.Manufacturer;
	model = devinfo_resp.Model;
	serial_number = devinfo_resp.SerialNumber;
	hardwareid = devinfo_resp.HardwareId;
	firmware_version = devinfo_resp.FirmwareVersion;
	{
		printf("\n================= + dump_tds__GetDeviceInformationResponse  start + >>>\n");
		printf("Manufacturer:%s\n", devinfo_resp.Manufacturer);
		printf("Model:%s\n", devinfo_resp.Model);
		printf("Serial Number:%s\n", devinfo_resp.SerialNumber);
		printf("Hardware Id:%s\n", devinfo_resp.HardwareId);
		printf("Firmware Version:%s\n", devinfo_resp.FirmwareVersion);
		printf("\n================= - dump_tds__GetDeviceInformationResponse end- <<<\n");
	}
	if (NULL != soap) soapFree(soap);
	return result == SOAP_OK;
}

bool MediaFinderOnvif::getOnvifRtsp(const char* device, 
	char* main_rtsp, char* sub_rtsp,
	const char* user, const char* pass)
{
	int result = -1;
	if (!device || !main_rtsp || !sub_rtsp)return -1;
	char media_url[1024] = { 0 };
	char main_token[1024] = { 0 };
	char sub_token[1024] = { 0 };
	LOG("start get stream url\n");
	std::shared_ptr<soap> sp_soap(soapNew(param.timeout_sec), [](soap* p_soap) {soapFree(p_soap); });
	if (!sp_soap) return false;
	soap_wsse_add_UsernameTokenDigest(sp_soap.get(), NULL, user, pass);
	{
		//获取 mediaurl
		struct _tds__GetCapabilities req;
		memset(&req, 0, sizeof(req));
		struct _tds__GetCapabilitiesResponse resp;
		memset(&resp, 0, sizeof(resp));
		//获取能力
		result = soap_call___tds__GetCapabilities(sp_soap.get(), device, NULL,
			&req, &resp
		);
		SOAP_CHECK_ERROR(result, sp_soap.get(), "soap_call___tds__GetCapabilities to error");
		strcpy(media_url, resp.Capabilities->Media->XAddr);
	}
	{
		//获取main_token sub_token 
		struct _trt__GetProfiles    req;
		struct _trt__GetProfilesResponse resp;
		memset(&req, 0, sizeof(req));
		memset(&resp, 0, sizeof(resp));

		result = soap_call___trt__GetProfiles(
			sp_soap.get(),
			media_url,
			NULL, //action
			&req, &resp
		);
		SOAP_CHECK_ERROR(result, sp_soap.get(), "soap_call___trt__GetProfiles to failed!\n");
		//主码流token
		if (resp.__sizeProfiles >= 1) strcpy(main_token, resp.Profiles[0].token);
		//辅码流token
		if (resp.__sizeProfiles > 1) strcpy(sub_token, resp.Profiles[1].token);
	}
	{
		//获取主码流
		struct _trt__GetStreamUri    req;
		struct _trt__GetStreamUriResponse resp;
		memset(&req, 0, sizeof(req));
		memset(&resp, 0, sizeof(resp));
		//请求的设置
		struct tt__StreamSetup    setup;
		struct tt__Transport      transport;
		memset(&setup, 0, sizeof(setup));
		memset(&transport, 0, sizeof(transport));
		setup.Stream = tt__StreamType__RTP_Unicast;//单播
		setup.Transport = &transport;
		setup.Transport->Protocol = tt__TransportProtocol__RTSP; //协议
		req.StreamSetup = &setup;
		req.ProfileToken = (char*)main_token;

		result = soap_call___trt__GetStreamUri(sp_soap.get(), media_url,
			NULL, //action 
			&req,
			&resp
		);
		SOAP_CHECK_ERROR(result, sp_soap.get(), "soap_call___trt__GetStreamUri to failed!\n");
		strcpy(main_rtsp, resp.MediaUri->Uri);
	}
	{
		//获取辅码流
		struct _trt__GetStreamUri    req;
		struct _trt__GetStreamUriResponse resp;
		memset(&req, 0, sizeof(req));
		memset(&resp, 0, sizeof(resp));
		//请求的设置
		struct tt__StreamSetup    setup;
		struct tt__Transport      transport;
		memset(&setup, 0, sizeof(setup));
		memset(&transport, 0, sizeof(transport));
		setup.Stream = tt__StreamType__RTP_Unicast;//单播
		setup.Transport = &transport;
		setup.Transport->Protocol = tt__TransportProtocol__RTSP; //协议
		req.StreamSetup = &setup;
		req.ProfileToken = (char*)sub_token;

		result = soap_call___trt__GetStreamUri(sp_soap.get(), media_url,
			NULL, //action 
			&req,
			&resp
		);
		SOAP_CHECK_ERROR(result, sp_soap.get(), "soap_call___trt__GetStreamUri to failed!\n");
		strcpy(sub_rtsp, resp.MediaUri->Uri);
	}
	return result == SOAP_OK;
}


