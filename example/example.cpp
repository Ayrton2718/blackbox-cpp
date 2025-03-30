#define MCAP_IMPLEMENTATION
#include <mcap/writer.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <google/protobuf/descriptor.pb.h>
#include "foxglove/Log.pb.h"

#include "blackbox.hpp"

// sample main function of logger using blackbox
int main()
{
    // create blackbox node
    blackbox::BlackBoxNode bb_node("ns", "name", blackbox::debug_mode_t::DEBUG);

    // create logger
    blackbox::LogRecorder::Logger logger;
    logger.init(&bb_node, blackbox::log_type_t::INFO, "tag_name");

    blackbox::Recorder<foxglove::Log> record;

    // log
    TAGGER(&logger, "test log");
    TAGGER(&logger, "test log %d", 1);
    TAGGER(&logger, "test log %s", "string");
    TAGGER(&logger, "test log %d", 100);
}
