#pragma once

#include "bb_blackbox.hpp"
#include "bb_logger.hpp"

namespace blackbox
{

class BlackBoxNode : public BlackBox, public LogRecorder
{
public:
    BlackBoxNode(std::string ns, std::string name, debug_mode_t debug_mode, const std::string& node_name, const std::string& name_space = "", 
                bool enable_update=false, storage_profile_t storage_profile=storage_profile_t::zstd_fast, uint64_t max_cache_size=1024*128)
        :   BlackBox(ns, name, debug_mode, "blackbox", storage_profile, max_cache_size),
            LogRecorder(this)
    {
    }
};

}