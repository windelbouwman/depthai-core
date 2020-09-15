#include <math.h>

#include <iostream>

#include "disparity_stream_post_processor.hpp"
#include "depthai-shared/disparity_luts.hpp"
#include "depthai-shared/metadata/frame_metadata.hpp"

DisparityStreamPostProcessor::DisparityStreamPostProcessor(
    bool produce_d_color
)
    : _produce_depth_color(produce_d_color)
{

}



void  DisparityStreamPostProcessor::onNewData(
    const StreamInfo &data_info,
    const StreamData &data
)
{
    assert(data_info.name == c_stream_in);

    if (_produce_depth_color)
    {
        prepareDepthColorAndNotifyObservers(data_info, data);
    }
}

void DisparityStreamPostProcessor::prepareDepthColorAndNotifyObservers(
    const StreamInfo &data_info,
    const StreamData &data
)
{
    StreamInfo depth_si(c_stream_out_color.c_str(),
            data_info.dimensions[0] * data_info.dimensions[1] * 3 + sizeof(FrameMetadata),
            {data_info.dimensions[0], data_info.dimensions[1], 3});

    std::vector<unsigned char> depth_raw(depth_si.size);
    const unsigned char* disp_uc = (const unsigned char*) data.data;

    for (int i = 0, j = 0; i < data.size - sizeof(FrameMetadata); ++i, j+=3)
    {
        const unsigned char &disp = *(disp_uc + i);
        depth_raw[j  ] = c_disp_to_color[disp][0];
        depth_raw[j+1] = c_disp_to_color[disp][1];
        depth_raw[j+2] = c_disp_to_color[disp][2];
    }
    FrameMetadata *m = (FrameMetadata *)(depth_raw.data() + depth_raw.size() - sizeof(FrameMetadata));
    memcpy(m, disp_uc + data.size - sizeof(FrameMetadata), sizeof(FrameMetadata));
    m->frameSize = 3 * (data.size - sizeof(FrameMetadata));

    StreamData depth_d;
    depth_d.packet_number = data.packet_number;
    depth_d.data = depth_raw.data();
    depth_d.size = depth_raw.size();

    notifyObservers(depth_si, depth_d);
}