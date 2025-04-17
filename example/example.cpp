#define MCAP_IMPLEMENTATION
#include <mcap/writer.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <google/protobuf/descriptor.pb.h>

#include "blackbox/blackbox.hpp"

#include "foxglove/Log.pb.h"
#include "foxglove/FrameTransform.pb.h"
#include "foxglove/SceneUpdate.pb.h"
#include "foxglove/LaserScan.pb.h"

#include "simpleproto/MultiArray.pb.h"

#define BOX_SIZE (5.0)

void set_point(foxglove::Point3* point, double x, double y)
{
    point->set_x(x);
    point->set_y(y);
    point->set_z(0.0);
}


void set_wall(foxglove::LinePrimitive* line)
{
    line->set_type(foxglove::LinePrimitive::LINE_LIST);

    // Set Pose
    foxglove::Pose* pose = line->mutable_pose();
    
    // Set position (Vector3)
    foxglove::Vector3* position = pose->mutable_position();
    position->set_x(0.0);
    position->set_y(0.0);
    position->set_z(0.0);

    // Set orientation (Quaternion)
    foxglove::Quaternion* orientation = pose->mutable_orientation();
    orientation->set_x(0.0);
    orientation->set_y(0.0);
    orientation->set_z(0.0);
    orientation->set_w(1.0);

    line->set_thickness(5);  // Set line thickness
    line->set_scale_invariant(true);  // Set scale invariant

    auto points = line->mutable_points();

    // first line
    set_point(line->add_points(), BOX_SIZE, BOX_SIZE);
    set_point(line->add_points(), BOX_SIZE, -BOX_SIZE);
    // second line
    set_point(line->add_points(), BOX_SIZE, -BOX_SIZE);
    set_point(line->add_points(), -BOX_SIZE, -BOX_SIZE);
    // third line
    set_point(line->add_points(), -BOX_SIZE, -BOX_SIZE);
    set_point(line->add_points(), -BOX_SIZE, BOX_SIZE);
    // fourth line
    set_point(line->add_points(), -BOX_SIZE, BOX_SIZE);
    set_point(line->add_points(), BOX_SIZE, BOX_SIZE);

    // Set Color
    foxglove::Color* color = line->mutable_color();
    color->set_r(0);  // Red
    color->set_g(0);    // Green
    color->set_b(255);    // Blue
    color->set_a(255);  // Alpha (fully opaque)
}

std::unique_ptr<simpleproto::MultiArrayDouble> create_multi_array_msg(double x, double y, double z)
{
    std::unique_ptr<simpleproto::MultiArrayDouble> array_msg = std::make_unique<simpleproto::MultiArrayDouble>();
    // Set example double array
    array_msg->add_values(x);
    array_msg->add_values(y);
    array_msg->add_values(z);
    return std::move(array_msg);
}

std::unique_ptr<foxglove::FrameTransform> create_frame_transform_msg(double x, double y, double z)
{
    std::unique_ptr<foxglove::FrameTransform> transform_msg = std::make_unique<foxglove::FrameTransform>();

    blackbox::set_proto_timestamp(transform_msg->mutable_timestamp());

    transform_msg->set_parent_frame_id("map");
    transform_msg->set_child_frame_id("base_link");

    // Set translation
    foxglove::Vector3* translation = transform_msg->mutable_translation();
    translation->set_x(x);
    translation->set_y(y);
    translation->set_z(z);

    // Set rotation (Quaternion)
    foxglove::Quaternion* rotation = transform_msg->mutable_rotation();
    rotation->set_x(0.0);
    rotation->set_y(0.0);
    rotation->set_z(0.0);
    rotation->set_w(1.0);

    return std::move(transform_msg);
}

std::unique_ptr<foxglove::SceneUpdate> create_scene_update_msg(double x, double y, double z)
{
    std::unique_ptr<foxglove::SceneUpdate> scene_update_msg = std::make_unique<foxglove::SceneUpdate>();
    
    auto element = scene_update_msg->add_entities();
    blackbox::set_proto_timestamp(element->mutable_timestamp());
    element->set_frame_id("map");
    element->set_id("wall");
    blackbox::set_proto_duaration(element->mutable_lifetime(), 0, 0);
    element->set_frame_locked(false);

    auto lines = element->mutable_lines();
    set_wall(lines->Add());

    return std::move(scene_update_msg);
}

std::unique_ptr<foxglove::LaserScan> create_laser_scan_msg(void)
{
    std::unique_ptr<foxglove::LaserScan> laser_scan_msg = std::make_unique<foxglove::LaserScan>();

    blackbox::set_proto_timestamp(laser_scan_msg->mutable_timestamp());

    laser_scan_msg->set_frame_id("base_link");

    // pose
    foxglove::Pose* pose = laser_scan_msg->mutable_pose();
    // Set position (Vector3)
    foxglove::Vector3* position = pose->mutable_position();
    position->set_x(0.0);
    position->set_y(0.0);
    position->set_z(0.0);
    // Set orientation (Quaternion)
    foxglove::Quaternion* orientation = pose->mutable_orientation();
    orientation->set_x(0.0);
    orientation->set_y(0.0);
    orientation->set_z(0.0);
    orientation->set_w(1.0);

    laser_scan_msg->set_start_angle(-1.57);
    laser_scan_msg->set_end_angle(1.57);

    // Add ranges
    for (double angle = -1.57; angle <= 1.57; angle += (1.0 * M_PI / 180.0)) {
        laser_scan_msg->add_ranges(4.0);
        laser_scan_msg->add_intensities(100);
    }
    return std::move(laser_scan_msg);
}


// sample main function of logger using blackbox
int main()
{
    // create blackbox node
    blackbox::BlackBox bb("ns", "name", blackbox::debug_mode_t::DEBUG);

    // create logger
    blackbox::Logger info;
    info.init(&bb, blackbox::log_type_t::INFO, "position");

    blackbox::Logger error;
    error.init(&bb, blackbox::log_type_t::ERR, "over_position");

    blackbox::Record<foxglove::FrameTransform> frame_record;
    frame_record.init(&bb, "tf");

    blackbox::Record<foxglove::SceneUpdate> scene_record;
    scene_record.init(&bb, "scene");

    blackbox::Record<foxglove::LaserScan> laser_record;
    laser_record.init(&bb, "laser_scan");

    blackbox::Record<simpleproto::MultiArrayDouble> array_record;
    array_record.init(&bb, "multi_array");

    blackbox::Record<simpleproto::MultiArrayBool> diag_record;
    diag_record.init(&bb, "diag");

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(10);  // Run for 10 seconds
    
    std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() < end_time) {
        // circular motion
        double x = 4.0 * std::cos(std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count());
        double y = 5.2 * std::sin(std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count());
        double z = 0.0;

        auto frame_msg = create_frame_transform_msg(x, y, z);
        auto scene_msg = create_scene_update_msg(x, y, z);
        auto array_msg = create_multi_array_msg(x, y, z);
        auto laser_msg = create_laser_scan_msg();

        // Record messages
        frame_record.record(frame_msg.get());
        scene_record.record(scene_msg.get());
        array_record.record(array_msg.get());
        laser_record.record(laser_msg.get());

        auto diag_msg = std::make_shared<simpleproto::MultiArrayBool>();
        // Is x in box
        diag_msg->add_values(x > -BOX_SIZE && x < BOX_SIZE);
        // Is y in box
        diag_msg->add_values(y > -BOX_SIZE && y < BOX_SIZE);

        if(!diag_msg->values(0) || !diag_msg->values(1))
        {
            TAGGER(&error, "Out of box: Position (%.2f, %.2f, %.2f)", x, y, z);
        }
        else
        {
            TAGGER(&info, "In box: Position (%.2f, %.2f, %.2f)", x, y, z);
        }
        diag_record.record(diag_msg);

        // Wait 1 second before next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
