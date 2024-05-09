#include "media_finder_monitor.h"

MediaFinderMonitor::MediaFinderMonitor()
{
}

MediaFinderMonitor::~MediaFinderMonitor()
{
	stop();
}

bool MediaFinderMonitor::init(const ParamMediaFinderMonitor& param_in,
							  const std::shared_ptr<MediaFinderOnvif>& sp_finder_in,
							  const std::function<void(const std::vector<ParamMediaSourceInfo>&, const std::vector<ParamMediaSourceInfo>&)>& callback_media_source_info_in)
{
	param = param_in;
	sp_finder = sp_finder_in;
	if (!sp_finder) return false;
	callback_media_source_info = callback_media_source_info_in;
	flag_exit = false;
	thread = std::thread(std::bind(&MediaFinderMonitor::threadFunc, this));
	printf("MediaFinderMonitor init done...\n");
	return true;
}

void MediaFinderMonitor::stop()
{
	flag_exit = true;
	if (thread.joinable()) thread.join();
}

void MediaFinderMonitor::compareVectors(const std::vector<ParamMediaSourceInfo>& vec_media_last,
	const std::vector<ParamMediaSourceInfo>& vec_media_this,
	std::vector<ParamMediaSourceInfo>& vec_media_new,
	std::vector<ParamMediaSourceInfo>& vec_media_miss)
{
	vec_media_new.clear();
	vec_media_miss.clear();
	if (vec_media_last.empty() || vec_media_this.empty()) return;
	for (const auto& item : vec_media_last)
	{
		const auto& find_it = std::find_if(vec_media_this.begin(), vec_media_this.end(), [=](const ParamMediaSourceInfo& this_info) 
			{	return item == this_info; });
		if (find_it == vec_media_this.end())
			vec_media_miss.push_back(*find_it);
	}
	for (const auto& item : vec_media_this)
	{
		const auto& find_it = std::find_if(vec_media_last.begin(), vec_media_last.end(), [=](const ParamMediaSourceInfo& last_info)
			{	return item == last_info; });
		if (find_it == vec_media_last.end())
			vec_media_new.push_back(*find_it);
	}
}

void MediaFinderMonitor::threadFunc()
{
	std::vector<ParamMediaSourceInfo> vec_media_last;
	std::vector<ParamMediaSourceInfo> vec_media_this;
	std::vector<ParamMediaSourceInfo> vec_media_new;
	std::vector<ParamMediaSourceInfo> vec_media_miss;
	while (!flag_exit)
	{
		do
		{
			if (!sp_finder->findMedia(vec_media_this)) break;
			compareVectors(vec_media_last, vec_media_this, vec_media_new, vec_media_miss);
			if (callback_media_source_info) callback_media_source_info(vec_media_new, vec_media_miss);
		} while (false);
		vec_media_last.swap(vec_media_this);// 将本次的结果设置为last，供下一次使用      
		std::this_thread::sleep_for(std::chrono::milliseconds(param.timeinterval_ms));
	}
}


