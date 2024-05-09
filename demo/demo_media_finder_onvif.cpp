#include "../media_finder_onvif.h"
#include <iostream>
#include <vector>
#include <thread>

#if 1
int main()
{
    ParamMediaFinderOnvif param_onvif_finder;
    MediaFinderOnvif onvif_finder;
    std::vector<ParamMediaSourceInfo> vec_media;

    if (!onvif_finder.init(param_onvif_finder)) return -1;
    for (size_t i = 0; i < 3; i++)
    {
        if (!onvif_finder.findMedia(vec_media)) return -1;
        for (const auto& item : vec_media)
        {
            item.debug();
        }
    }
    printf("demo_media_finder done!\n");
    return 0;
}
#endif