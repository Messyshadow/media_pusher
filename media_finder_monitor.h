#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include "media_finder_onvif.h"

struct ParamMediaFinderMonitor
{
	uint32_t timeinterval_ms = 1000 * 60 * 5;// 5���Ӽ��һ��
};
class MediaFinderMonitor
{
public:
	MediaFinderMonitor();
	~MediaFinderMonitor();
	bool init(
		const ParamMediaFinderMonitor& param_in,
		const std::shared_ptr<MediaFinderOnvif>& sp_finder_in,    // ����಻��finder������������ֻ���ã����finder���ⲿ�����õĳ�ʼ���õ�
		const std::function<void(const std::vector<ParamMediaSourceInfo>&, const std::vector<ParamMediaSourceInfo>&)>& callback_media_source_info_in = nullptr
	);
	void stop();

protected:
	void threadFunc();

private:
	void compareVectors(
		const std::vector<ParamMediaSourceInfo>& vec_media_last,
		const std::vector<ParamMediaSourceInfo>& vec_media_this,
		std::vector<ParamMediaSourceInfo>& vec_media_new,
		std::vector<ParamMediaSourceInfo>& vec_media_miss);

private:
	bool flag_exit = false;
	std::thread thread;
	ParamMediaFinderMonitor param;
	std::shared_ptr<MediaFinderOnvif> sp_finder;
	std::function<void(const std::vector<ParamMediaSourceInfo>&, const std::vector<ParamMediaSourceInfo>&)> callback_media_source_info;
};

