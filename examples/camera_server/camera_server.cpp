#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <filesystem>
#include <string>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/camera_server/camera_server.h>
#include <mavsdk/plugins/param_server/param_server.h>

int main(int argc, char** argv)
{
    mavsdk::Mavsdk mavsdk;
    mavsdk::Mavsdk::Configuration configuration(mavsdk::Mavsdk::Configuration::UsageType::Camera);
    mavsdk.set_configuration(configuration);

    // 14030 is the default camera port for PX4 SITL
    auto result = mavsdk.add_any_connection("udp://127.0.0.1:14030");
    if (result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Could not establish connection: " << result << std::endl;
        return 1;
    }
    std::cout << "Created camera server connection" << std::endl;

    auto server_component =
        mavsdk.server_component_by_type(mavsdk::Mavsdk::ServerComponentType::Camera);
    auto camera_server = mavsdk::CameraServer{server_component};
    auto param_server = mavsdk::ParamServer{server_component};

    param_server.provide_param_custom("CAM_EV", "1.0", mavsdk::ParamServer::ValueType::FloatType);
    param_server.subscribe_param_changed_async(
        "CAM_EV", mavsdk::ParamServer::ValueType::FloatType, [](std::string value) {
            std::cout << "change CAM_EV to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_CUSTOMWB", "5500", mavsdk::ParamServer::ValueType::Uint16Type);
    param_server.subscribe_param_changed_async(
        "CAM_CUSTOMWB", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_CUSTOMWB to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_SPOTAREA", "0", mavsdk::ParamServer::ValueType::Uint16Type);
    param_server.subscribe_param_changed_async(
        "CAM_SPOTAREA", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_SPOTAREA to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_ASPECTRATIO", "1.777777", mavsdk::ParamServer::ValueType::FloatType);
    param_server.subscribe_param_changed_async(
        "CAM_ASPECTRATIO", mavsdk::ParamServer::ValueType::FloatType, [](std::string value) {
            std::cout << "change CAM_ASPECTRATIO to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_PHOTOQUAL", "1", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_PHOTOQUAL", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_PHOTOQUAL to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_FILENUMOPT", "0", mavsdk::ParamServer::ValueType::Uint8Type);
    param_server.subscribe_param_changed_async(
        "CAM_FILENUMOPT", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_FILENUMOPT to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_PHOTOFMT", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_PHOTOFMT", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_PHOTOFMT to " << value << std::endl;
        });

    param_server.provide_param_custom("CAM_MODE", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_MODE", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_MODE to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_FLICKER", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_FLICKER", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_FLICKER to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_SHUTTERSPD", "0.01", mavsdk::ParamServer::ValueType::FloatType);
    param_server.subscribe_param_changed_async(
        "CAM_SHUTTERSPD", mavsdk::ParamServer::ValueType::FloatType, [](std::string value) {
            std::cout << "change CAM_SHUTTERSPD to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_WBMODE", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_WBMODE", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_WBMODE to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_COLORENCODE", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_COLORENCODE", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_COLORENCODE to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_EXPMODE", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_EXPMODE", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_EXPMODE to " << value << std::endl;
        });

    param_server.provide_param_custom("CAM_ISO", "100", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_ISO", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_ISO to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_VIDRES", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_VIDRES", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_VIDRES to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_IMAGEDEWARP", "0", mavsdk::ParamServer::ValueType::Uint8Type);
    param_server.subscribe_param_changed_async(
        "CAM_IMAGEDEWARP", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_IMAGEDEWARP to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_PHOTORATIO", "1", mavsdk::ParamServer::ValueType::Uint8Type);
    param_server.subscribe_param_changed_async(
        "CAM_PHOTORATIO", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_PHOTORATIO to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_VIDFMT", "1", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_VIDFMT", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_VIDFMT to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_METERING", "0", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_METERING", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_METERING to " << value << std::endl;
        });

    param_server.provide_param_custom(
        "CAM_COLORMODE", "1", mavsdk::ParamServer::ValueType::Uint32Type);
    param_server.subscribe_param_changed_async(
        "CAM_COLORMODE", mavsdk::ParamServer::ValueType::Uint16Type, [](std::string value) {
            std::cout << "change CAM_COLORMODE to " << value << std::endl;
        });

    // First add all subscriptions. This defines the camera capabilities.
    bool is_capture_in_progress = false;
    int32_t image_count = 0;
    camera_server.subscribe_take_photo(
        [&camera_server, &is_capture_in_progress, &image_count](int32_t index) {
            std::cout << "taking a picture (" << +index << ")..." << std::endl;

            is_capture_in_progress = true;
            // TODO : actually capture image here
            // simulating with delay
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // TODO: populate with telemetry data
            auto position = mavsdk::CameraServer::Position{};
            auto attitude = mavsdk::CameraServer::Quaternion{};

            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::system_clock::now().time_since_epoch())
                                 .count();
            auto success = true;
            camera_server.respond_take_photo(
                mavsdk::CameraServer::TakePhotoFeedback::Ok,
                mavsdk::CameraServer::CaptureInfo{
                    .position = position,
                    .attitude_quaternion = attitude,
                    .time_utc_us = static_cast<uint64_t>(timestamp),
                    .is_success = success,
                    .index = index,
                    .file_url = {},
                });
            is_capture_in_progress = false;
            image_count++;
        });

    std::chrono::steady_clock::time_point start_video_time;
    bool is_recording_video = false;
    camera_server.subscribe_start_video(
        [&start_video_time, &is_recording_video](int32_t stream_id) {
            std::cout << "Start video record" << std::endl;
            is_recording_video = true;
            start_video_time = std::chrono::steady_clock::now();
        });

    camera_server.subscribe_stop_video([&is_recording_video](int32_t stream_id) {
        std::cout << "Stop video record" << std::endl;
        is_recording_video = false;
    });

    camera_server.subscribe_start_video_streaming(
        [](int32_t stream_id) { std::cout << "Start video streaming" << std::endl; });

    camera_server.subscribe_stop_video_streaming(
        [](int32_t stream_id) { std::cout << "Stop video streaming" << std::endl; });

    camera_server.subscribe_set_camera_mode([](mavsdk::CameraServer::CameraMode camera_mode) {
        std::cout << "Set camera mode " << camera_mode << std::endl;
    });

    camera_server.subscribe_storage_information([&camera_server](int32_t storage_id) {
        mavsdk::CameraServer::StorageInformation storage_information;
        constexpr int kTotalStorage = 4 * 1024 * 1024;
        storage_information.total_storage_mib = kTotalStorage;
        storage_information.used_storage_mib = 100;
        storage_information.available_storage_mib =
            kTotalStorage - storage_information.used_storage_mib;
        storage_information.storage_status =
            mavsdk::CameraServer::StorageInformation::StorageStatus::Formatted;
        storage_information.storage_type =
            mavsdk::CameraServer::StorageInformation::StorageType::Microsd;

        camera_server.respond_storage_information(storage_information);
    });

    camera_server.subscribe_capture_status([&camera_server,
                                            &image_count,
                                            &is_capture_in_progress,
                                            &is_recording_video,
                                            &start_video_time](int32_t reserved) {
        mavsdk::CameraServer::CaptureStatus capture_status;
        capture_status.image_count = image_count;
        capture_status.image_status =
            is_capture_in_progress ?
                mavsdk::CameraServer::CaptureStatus::ImageStatus::CaptureInProgress :
                mavsdk::CameraServer::CaptureStatus::ImageStatus::Idle;
        capture_status.video_status =
            is_recording_video ?
                mavsdk::CameraServer::CaptureStatus::VideoStatus::CaptureInProgress :
                mavsdk::CameraServer::CaptureStatus::VideoStatus::Idle;
        auto current_time = std::chrono::steady_clock::now();
        if (is_recording_video) {
            capture_status.recording_time_s =
                std::chrono::duration_cast<std::chrono::seconds>(current_time - start_video_time)
                    .count();
        }
        camera_server.respond_capture_status(capture_status);
    });

    camera_server.subscribe_format_storage([](int storage_id) {
        std::cout << "format storage with id : " << storage_id << std::endl;
    });

    camera_server.subscribe_reset_settings(
        [](int camera_id) { std::cout << "reset camera settings" << std::endl; });
    // Then set the initial state of everything.

    // Finally call set_information() to "activate" the camera plugin.
    auto ret = camera_server.set_information({
        .vendor_name = "MAVSDK",
        .model_name = "Example Camera Server",
        .firmware_version = "1.0.0",
        .focal_length_mm = 3.0,
        .horizontal_sensor_size_mm = 3.68,
        .vertical_sensor_size_mm = 2.76,
        .horizontal_resolution_px = 3280,
        .vertical_resolution_px = 2464,
        .lens_id = 0,
        .definition_file_version = 1,
        .definition_file_uri = "ftp://90.xml", // for mavlink ftp test
    });

    if (ret != mavsdk::CameraServer::Result::Success) {
        std::cerr << "Failed to set camera info, exiting" << std::endl;
        return 2;
    }

    // works as a server and never quit
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
