#pragma once


#include <google/protobuf/descriptor.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

namespace blackbox
{

using bb_time_t = struct timespec;

inline bb_time_t get_bb_tim(void){
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

inline void set_proto_timestamp(google::protobuf::Timestamp* stamp, bb_time_t tim = get_bb_tim())
{
    stamp->set_seconds(tim.tv_sec);
    stamp->set_nanos(tim.tv_nsec);
}

inline void set_proto_duaration(google::protobuf::Duration* stamp, int64_t sec, int32_t nsec)
{
    stamp->set_seconds(sec);
    stamp->set_nanos(nsec);
}

}