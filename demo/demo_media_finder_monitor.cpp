#include "../media_finder_monitor.h"
#include "../media_finder_onvif.h"
#include "../media_finder_monitor.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
	ParamMediaFinderOnvif param_onvif_finder;
	std::shared_ptr<MediaFinderOnvif> sp_onvif_finder = std::make_shared<MediaFinderOnvif>();
	std::vector<ParamMediaSourceInfo> vec_media;

	if (!sp_onvif_finder->init(param_onvif_finder)) return -1;
	auto callback_mediainfo = [](const std::vector<ParamMediaSourceInfo>& vec_init_media_info, 
		const std::vector<ParamMediaSourceInfo>& vec_new_media_info)
	{
		for (const auto& item : vec_init_media_info)
			item.debug();
		for (const auto& item : vec_new_media_info)
			item.debug();
		printf("callback_mediainfo call!\n");
	};
	ParamMediaFinderMonitor param_media_finder_monitor;
	param_media_finder_monitor.timeinterval_ms = 8000;
	MediaFinderMonitor media_finder_monitor ;
	if (!media_finder_monitor.init(param_media_finder_monitor, sp_onvif_finder, callback_mediainfo))
	{
		printf("media_finder_monitor init failed!\n");
		return -1;
	}
	while (true)
	{
		std::string msg;
		printf("input msg:");
		std::cin >> msg;
		
		if (msg == "q")
		{
			break;
		}
	}
	printf("demo_media_finder_monitor is end!\n");
	getchar();
	return 0;
}