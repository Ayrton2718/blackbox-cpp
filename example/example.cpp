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
#include "geometry_msgs/PoseWithCovarianceStamped.pb.h"

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

std::shared_ptr<simpleproto::MultiArrayDouble> create_multi_array_msg(double x, double y, double z)
{
    std::shared_ptr<simpleproto::MultiArrayDouble> array_msg = std::make_shared<simpleproto::MultiArrayDouble>();
    // Set example double array
    array_msg->add_values(x);
    array_msg->add_values(y);
    array_msg->add_values(z);
    return array_msg;
}

std::shared_ptr<foxglove::FrameTransform> create_frame_transform_msg(double x, double y, double z)
{
    std::shared_ptr<foxglove::FrameTransform> transform_msg = std::make_shared<foxglove::FrameTransform>();

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

    return transform_msg;
}

std::shared_ptr<foxglove::SceneUpdate> create_scene_update_msg(double x, double y, double z)
{
    std::shared_ptr<foxglove::SceneUpdate> scene_update_msg = std::make_shared<foxglove::SceneUpdate>();
    
    auto element = scene_update_msg->add_entities();
    blackbox::set_proto_timestamp(element->mutable_timestamp());
    element->set_frame_id("map");
    element->set_id("wall");
    blackbox::set_proto_duaration(element->mutable_lifetime(), 0, 0);
    element->set_frame_locked(false);

    auto lines = element->mutable_lines();
    set_wall(lines->Add());

    return scene_update_msg;
}

std::shared_ptr<geometry_msgs::PoseWithCovarianceStamped> create_localization_msg(double x, double y, double yaw)
{
    auto msg = std::make_shared<geometry_msgs::PoseWithCovarianceStamped>();

    // Set header
    auto header = msg->mutable_header();
    auto stamp = header->mutable_stamp();
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    stamp->set_sec(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    stamp->set_nanosec(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000);
    header->set_frame_id("map");

    // Set pose
    auto pose_with_cov = msg->mutable_pose();
    auto pose = pose_with_cov->mutable_pose();
    
    // Set position
    auto position = pose->mutable_position();
    position->set_x(x);
    position->set_y(y);
    position->set_z(0.0);

    // Set orientation (quaternion from yaw angle)
    auto orientation = pose->mutable_orientation();
    orientation->set_x(0.0);
    orientation->set_y(0.0);
    orientation->set_z(std::sin(yaw / 2.0));
    orientation->set_w(std::cos(yaw / 2.0));

    // Set covariance matrix (6x6 = 36 elements)
    // Diagonal elements represent variance for x, y, z, roll, pitch, yaw
    // Using small values to indicate high confidence in localization
    for (int i = 0; i < 36; i++) {
        pose_with_cov->add_covariance(0.0);
    }
    // Set diagonal elements (variance)
    pose_with_cov->set_covariance(0, 0.01);   // x variance
    pose_with_cov->set_covariance(7, 0.01);   // y variance
    pose_with_cov->set_covariance(14, 0.01);  // z variance
    pose_with_cov->set_covariance(21, 0.001); // roll variance
    pose_with_cov->set_covariance(28, 0.001); // pitch variance
    pose_with_cov->set_covariance(35, 0.01);  // yaw variance

    return msg;
}

std::shared_ptr<foxglove::LaserScan> create_laser_scan_msg(void)
{
    std::shared_ptr<foxglove::LaserScan> laser_scan_msg = std::make_shared<foxglove::LaserScan>();

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
    return laser_scan_msg;
}


// sample main function of logger using blackbox
int main()
{
    auto bb = blackbox::BlackBox::create("ns", "name", blackbox::debug_mode_t::DEBUG);

    auto info = blackbox::Logger::create(bb, blackbox::log_type_t::INFO, "position");
    auto error = blackbox::Logger::create(bb, blackbox::log_type_t::ERR, "over_position");

    auto frame_record = blackbox::Record<foxglove::FrameTransform>::create(bb, "tf");
    auto scene_record = blackbox::Record<foxglove::SceneUpdate>::create(bb, "scene");
    auto laser_record = blackbox::Record<foxglove::LaserScan>::create(bb, "laser_scan");
    auto array_record = blackbox::Record<simpleproto::MultiArrayDouble>::create(bb, "multi_array");
    auto diag_record = blackbox::Record<simpleproto::MultiArrayBool>::create(bb, "diag");
    auto localization_record = blackbox::Record<geometry_msgs::PoseWithCovarianceStamped>::create(bb, "localization");

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(10);  // Run for 10 seconds
    
    std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() < end_time) {
        // circular motion
        double x = 4.0 * std::cos(std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count());
        double y = 5.2 * std::sin(std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count());
        double z = 0.0;

        // Calculate yaw from velocity direction (tangent to circular motion)
        double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count();
        double yaw = elapsed + M_PI / 2.0;  // Tangent direction for circular motion

        auto frame_msg = create_frame_transform_msg(x, y, z);
        auto scene_msg = create_scene_update_msg(x, y, z);
        auto array_msg = create_multi_array_msg(x, y, z);
        auto laser_msg = create_laser_scan_msg();
        auto localization_msg = create_localization_msg(x, y, yaw);

        // Record messages
        frame_record->record(frame_msg);
        scene_record->record(scene_msg);
        array_record->record(array_msg);
        laser_record->record(laser_msg);
        localization_record->record(localization_msg);

        auto diag_msg = std::make_shared<simpleproto::MultiArrayBool>();
        // Is x in box
        diag_msg->add_values(x > -BOX_SIZE && x < BOX_SIZE);
        // Is y in box
        diag_msg->add_values(y > -BOX_SIZE && y < BOX_SIZE);

        if(!diag_msg->values(0) || !diag_msg->values(1))
        {
            TAGGER(error, "Out of box: Position (%.2f, %.2f, %.2f)", x, y, z);
        }
        else
        {
            TAGGER(info, "In box: Position (%.2f, %.2f, %.2f)", x, y, z);
        }
        diag_record->record(diag_msg);

        // Wait 1 second before next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
