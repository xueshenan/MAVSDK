#pragma once

#include "plugins/camera_server/camera_server.h"
#include "server_plugin_impl_base.h"
#include "callback_list.h"

namespace mavsdk {

class CameraServerImpl : public ServerPluginImplBase {
public:
    explicit CameraServerImpl(std::shared_ptr<ServerComponent> server_component);
    ~CameraServerImpl() override;

    void init() override;
    void deinit() override;

    CameraServer::Result set_information(CameraServer::Information information);
    CameraServer::Result set_in_progress(bool in_progress);

    CameraServer::TakePhotoHandle
    subscribe_take_photo(const CameraServer::TakePhotoCallback& callback);
    void unsubscribe_take_photo(CameraServer::TakePhotoHandle handle);

    CameraServer::Result respond_take_photo(
        CameraServer::TakePhotoFeedback take_photo_feedback,
        CameraServer::CaptureInfo capture_info);

    CameraServer::StartVideoHandle
    subscribe_start_video(const CameraServer::StartVideoCallback& callback);
    void unsubscribe_start_video(CameraServer::StartVideoHandle handle);

    CameraServer::StopVideoHandle
    subscribe_stop_video(const CameraServer::StopVideoCallback& callback);
    void unsubscribe_stop_video(CameraServer::StopVideoHandle handle);

    CameraServer::StartVideoStreamingHandle
    subscribe_start_video_streaming(const CameraServer::StartVideoStreamingCallback& callback);
    void unsubscribe_start_video_streaming(CameraServer::StartVideoStreamingHandle handle);

    CameraServer::StopVideoStreamingHandle
    subscribe_stop_video_streaming(const CameraServer::StopVideoStreamingCallback& callback);
    void unsubscribe_stop_video_streaming(CameraServer::StopVideoStreamingHandle handle);

    CameraServer::SetCameraModeHandle
    subscribe_set_camera_mode(const CameraServer::SetCameraModeCallback& callback);
    void unsubscribe_set_camera_mode(CameraServer::SetCameraModeHandle handle);

    CameraServer::StorageInformationHandle
    subscribe_storage_information(const CameraServer::StorageInformationCallback& callback);
    void unsubscribe_storage_information(CameraServer::StorageInformationHandle handle);
    CameraServer::Result
    respond_storage_information(CameraServer::StorageInformation storage_information) const;

private:
    enum StatusFlags {
        IN_PROGRESS = 1 << 0,
        INTERVAL_SET = 1 << 1,
    };

    enum class TriggerControl {
        IGNORE = -1,
        DISABLE = 0,
        ENABLE = 1,
    };

    bool _is_information_set{};
    CameraServer::Information _information{};

    // CAMERA_CAPTURE_STATUS fields
    // TODO: how do we keep this info in sync between plugin instances?
    bool _is_image_capture_in_progress{};
    bool _is_image_capture_interval_set{};
    float _image_capture_timer_interval_s{};
    void* _image_capture_timer_cookie{};
    int32_t _image_capture_count{};

    CallbackList<int32_t> _take_photo_callbacks{};
    CallbackList<int32_t> _start_video_callbacks{};
    CallbackList<int32_t> _stop_video_callbacks{};
    CallbackList<int32_t> _start_video_streaming_callbacks{};
    CallbackList<int32_t> _stop_video_streaming_callbacks{};
    CallbackList<CameraServer::CameraMode> _set_camera_mode_callbacks{};
    CallbackList<int32_t> _storage_information_callbacks{};

    MavlinkCommandReceiver::CommandLong _last_take_photo_command;

    uint8_t _last_storage_id;

    bool parse_version_string(const std::string& version_str);
    bool parse_version_string(const std::string& version_str, uint32_t& version);
    void start_image_capture_interval(float interval, int32_t count, int32_t index);
    void stop_image_capture_interval();

    std::optional<mavlink_message_t>
    process_camera_information_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_camera_settings_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_storage_information_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_storage_format(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_camera_capture_status_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_reset_camera_settings(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_set_camera_mode(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_set_camera_zoom(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_set_camera_focus(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_set_storage_usage(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_image_start_capture(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_image_stop_capture(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_camera_image_capture_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_start_capture(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_stop_capture(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_start_streaming(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_stop_streaming(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_stream_information_request(const MavlinkCommandReceiver::CommandLong& command);
    std::optional<mavlink_message_t>
    process_video_stream_status_request(const MavlinkCommandReceiver::CommandLong& command);
};

} // namespace mavsdk
