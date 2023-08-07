#include "camera_server_impl.h"
#include "callback_list.tpp"

#include <thread> // FIXME: remove me

namespace mavsdk {

template class CallbackList<int32_t>;

CameraServerImpl::CameraServerImpl(std::shared_ptr<ServerComponent> server_component) :
    ServerPluginImplBase(server_component)
{
    _server_component_impl->register_plugin(this);
}

CameraServerImpl::~CameraServerImpl()
{
    _server_component_impl->unregister_plugin(this);
}

void CameraServerImpl::init()
{
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_CAMERA_INFORMATION,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_camera_information_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_CAMERA_SETTINGS,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_camera_settings_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_STORAGE_INFORMATION,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_storage_information_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_STORAGE_FORMAT,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_storage_format(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_camera_capture_status_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_RESET_CAMERA_SETTINGS,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_reset_camera_settings(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_SET_CAMERA_MODE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_set_camera_mode(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_SET_CAMERA_ZOOM,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_set_camera_zoom(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_SET_CAMERA_FOCUS,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_set_camera_focus(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_SET_STORAGE_USAGE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_set_storage_usage(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_IMAGE_START_CAPTURE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_image_start_capture(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_IMAGE_STOP_CAPTURE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_image_stop_capture(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_CAMERA_IMAGE_CAPTURE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_camera_image_capture_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_VIDEO_START_CAPTURE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_start_capture(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_VIDEO_STOP_CAPTURE,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_stop_capture(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_VIDEO_START_STREAMING,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_start_streaming(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_VIDEO_STOP_STREAMING,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_stop_streaming(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_stream_information_request(command);
        },
        this);
    _server_component_impl->register_mavlink_command_handler(
        MAV_CMD_REQUEST_VIDEO_STREAM_STATUS,
        [this](const MavlinkCommandReceiver::CommandLong& command) {
            return process_video_stream_status_request(command);
        },
        this);
}

void CameraServerImpl::deinit()
{
    stop_image_capture_interval();
    _server_component_impl->unregister_all_mavlink_command_handlers(this);
}

bool CameraServerImpl::parse_version_string(const std::string& version_str)
{
    uint32_t unused;

    return parse_version_string(version_str, unused);
}

bool CameraServerImpl::parse_version_string(const std::string& version_str, uint32_t& version)
{
    // empty string means no version
    if (version_str.empty()) {
        version = 0;

        return true;
    }

    uint8_t major{}, minor{}, patch{}, dev{};

    auto ret = sscanf(version_str.c_str(), "%hhu.%hhu.%hhu.%hhu", &major, &minor, &patch, &dev);

    if (ret == EOF) {
        return false;
    }

    // pack version according to MAVLINK spec
    version = dev << 24 | patch << 16 | minor << 8 | major;

    return true;
}

CameraServer::Result CameraServerImpl::set_information(CameraServer::Information information)
{
    if (!parse_version_string(information.firmware_version)) {
        LogDebug() << "incorrectly formatted firmware version string: "
                   << information.firmware_version;
        return CameraServer::Result::WrongArgument;
    }

    // TODO: validate information.definition_file_uri

    _is_information_set = true;
    _information = information;

    return CameraServer::Result::Success;
}

CameraServer::Result CameraServerImpl::set_video_stream_info(
    std::vector<CameraServer::VideoStreamInfo> video_stream_infos)
{
    _is_video_stream_info_set = true;
    _video_stream_infos = video_stream_infos;
    return CameraServer::Result::Success;
}

CameraServer::TakePhotoHandle
CameraServerImpl::subscribe_take_photo(const CameraServer::TakePhotoCallback& callback)
{
    return _take_photo_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_take_photo(CameraServer::TakePhotoHandle handle)
{
    _take_photo_callbacks.unsubscribe(handle);
}

CameraServer::Result CameraServerImpl::respond_take_photo(
    CameraServer::TakePhotoFeedback take_photo_feedback, CameraServer::CaptureInfo capture_info)
{
    // If capture_info.index == INT32_MIN, it means this was an interval
    // capture rather than a single image capture.
    if (capture_info.index != INT32_MIN) {
        // We expect each capture to be the next sequential number.
        // If _image_capture_count == 0, we ignore since it means that this is
        // the first photo since the plugin was initialized.
        if (_image_capture_count != 0 && capture_info.index != _image_capture_count + 1) {
            LogErr() << "unexpected image index, expecting " << +(_image_capture_count + 1)
                     << " but was " << +capture_info.index;
        }

        _image_capture_count = capture_info.index;
    }

    // REVISIT: Should we cache all CaptureInfo in memory for single image
    // captures so that we can respond to requests for lost CAMERA_IMAGE_CAPTURED
    // messages without calling back to user code?

    static const uint8_t camera_id = 0; // deprecated unused field

    const float attitude_quaternion[] = {
        capture_info.attitude_quaternion.w,
        capture_info.attitude_quaternion.x,
        capture_info.attitude_quaternion.y,
        capture_info.attitude_quaternion.z,
    };

    // There needs to be enough data to be copied mavlink internal.
    capture_info.file_url.resize(205);

    mavlink_message_t msg{};
    mavlink_msg_camera_image_captured_pack(
        _server_component_impl->get_own_system_id(),
        _server_component_impl->get_own_component_id(),
        &msg,
        static_cast<uint32_t>(_server_component_impl->get_time().elapsed_s() * 1e3),
        capture_info.time_utc_us,
        camera_id,
        static_cast<int32_t>(capture_info.position.latitude_deg * 1e7),
        static_cast<int32_t>(capture_info.position.longitude_deg * 1e7),
        static_cast<int32_t>(capture_info.position.absolute_altitude_m * 1e3f),
        static_cast<int32_t>(capture_info.position.relative_altitude_m * 1e3f),
        attitude_quaternion,
        capture_info.index,
        capture_info.is_success,
        capture_info.file_url.c_str());

    // TODO: this should be a broadcast message
    _server_component_impl->send_message(msg);
    LogDebug() << "sent camera image captured msg - index: " << +capture_info.index;

    return CameraServer::Result::Success;
}

CameraServer::StartVideoHandle
CameraServerImpl::subscribe_start_video(const CameraServer::StartVideoCallback& callback)
{
    return _start_video_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_start_video(CameraServer::StartVideoHandle handle)
{
    _start_video_callbacks.unsubscribe(handle);
}

CameraServer::StopVideoHandle
CameraServerImpl::subscribe_stop_video(const CameraServer::StopVideoCallback& callback)
{
    return _stop_video_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_stop_video(CameraServer::StopVideoHandle handle)
{
    return _stop_video_callbacks.unsubscribe(handle);
}

CameraServer::StartVideoStreamingHandle CameraServerImpl::subscribe_start_video_streaming(
    const CameraServer::StartVideoStreamingCallback& callback)
{
    return _start_video_streaming_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_start_video_streaming(
    CameraServer::StartVideoStreamingHandle handle)
{
    return _start_video_streaming_callbacks.unsubscribe(handle);
}

CameraServer::StopVideoStreamingHandle CameraServerImpl::subscribe_stop_video_streaming(
    const CameraServer::StopVideoStreamingCallback& callback)
{
    return _stop_video_streaming_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_stop_video_streaming(
    CameraServer::StopVideoStreamingHandle handle)
{
    return _stop_video_streaming_callbacks.unsubscribe(handle);
}

CameraServer::SetModeHandle
CameraServerImpl::subscribe_set_mode(const CameraServer::SetModeCallback& callback)
{
    return _set_mode_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_set_mode(CameraServer::SetModeHandle handle)
{
    _set_mode_callbacks.unsubscribe(handle);
}

CameraServer::StorageInformationHandle CameraServerImpl::subscribe_storage_information(
    const CameraServer::StorageInformationCallback& callback)
{
    return _storage_information_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_storage_information(
    CameraServer::StorageInformationHandle handle)
{
    _storage_information_callbacks.unsubscribe(handle);
}

CameraServer::Result CameraServerImpl::respond_storage_information(
    CameraServer::StorageInformation storage_information) const
{
    const uint8_t storage_count = 1;

    const float total_capacity = storage_information.total_storage_mib;
    const float used_capacity = storage_information.used_storage_mib;
    const float available_capacity = storage_information.available_storage_mib;
    const float read_speed = storage_information.read_speed;
    const float write_speed = storage_information.write_speed;

    auto status = STORAGE_STATUS::STORAGE_STATUS_NOT_SUPPORTED;
    switch (storage_information.storage_status) {
        case CameraServer::StorageInformation::StorageStatus::NotAvailable:
            status = STORAGE_STATUS::STORAGE_STATUS_NOT_SUPPORTED;
            break;
        case CameraServer::StorageInformation::StorageStatus::Unformatted:
            status = STORAGE_STATUS::STORAGE_STATUS_UNFORMATTED;
            break;
        case CameraServer::StorageInformation::StorageStatus::Formatted:
            status = STORAGE_STATUS::STORAGE_STATUS_READY;
            break;
        case CameraServer::StorageInformation::StorageStatus::NotSupported:
            status = STORAGE_STATUS::STORAGE_STATUS_NOT_SUPPORTED;
            break;
    }

    auto type = STORAGE_TYPE::STORAGE_TYPE_UNKNOWN;
    switch (storage_information.storage_type) {
        case CameraServer::StorageInformation::StorageType::UsbStick:
            type = STORAGE_TYPE::STORAGE_TYPE_USB_STICK;
            break;
        case CameraServer::StorageInformation::StorageType::Sd:
            type = STORAGE_TYPE::STORAGE_TYPE_SD;
            break;
        case CameraServer::StorageInformation::StorageType::Microsd:
            type = STORAGE_TYPE::STORAGE_TYPE_MICROSD;
            break;
        case CameraServer::StorageInformation::StorageType::Hd:
            type = STORAGE_TYPE::STORAGE_TYPE_HD;
            break;
        case CameraServer::StorageInformation::StorageType::Other:
            type = STORAGE_TYPE::STORAGE_TYPE_OTHER;
            break;
        default:
            break;
    }

    std::string name("");
    // This needs to be long enough, otherwise the memcpy in mavlink overflows.
    name.resize(32);
    const uint8_t storage_usage = 0;

    mavlink_message_t msg{};
    mavlink_msg_storage_information_pack(
        _server_component_impl->get_own_system_id(),
        _server_component_impl->get_own_component_id(),
        &msg,
        static_cast<uint32_t>(_server_component_impl->get_time().elapsed_s() * 1e3),
        _last_storage_id,
        storage_count,
        status,
        total_capacity,
        used_capacity,
        available_capacity,
        read_speed,
        write_speed,
        type,
        name.data(),
        storage_usage);

    _server_component_impl->send_message(msg);
    LogDebug() << "send storage information";
    return CameraServer::Result::Success;
}

CameraServer::CaptureStatusHandle
CameraServerImpl::subscribe_capture_status(const CameraServer::CaptureStatusCallback& callback)
{
    return _capture_status_callbacks.subscribe(callback);
}
void CameraServerImpl::unsubscribe_capture_status(CameraServer::CaptureStatusHandle handle)
{
    _capture_status_callbacks.unsubscribe(handle);
}

CameraServer::Result
CameraServerImpl::respond_capture_status(CameraServer::CaptureStatus capture_status) const
{
    uint8_t image_status{};
    if (capture_status.image_status ==
            CameraServer::CaptureStatus::ImageStatus::CaptureInProgress ||
        capture_status.image_status ==
            CameraServer::CaptureStatus::ImageStatus::IntervalInProgress) {
        image_status |= StatusFlags::IN_PROGRESS;
    }

    if (capture_status.image_status == CameraServer::CaptureStatus::ImageStatus::IntervalIdle ||
        capture_status.image_status ==
            CameraServer::CaptureStatus::ImageStatus::IntervalInProgress ||
        _is_image_capture_interval_set) {
        image_status |= StatusFlags::INTERVAL_SET;
    }

    uint8_t video_status = 0;
    if (capture_status.video_status == CameraServer::CaptureStatus::VideoStatus::Idle) {
        video_status = 0;
    } else if (
        capture_status.video_status ==
        CameraServer::CaptureStatus::VideoStatus::CaptureInProgress) {
        video_status = 1;
    }
    const uint32_t recording_time_ms =
        static_cast<uint32_t>(static_cast<double>(capture_status.recording_time_s) * 1e3);
    const float available_capacity = capture_status.available_capacity;

    mavlink_message_t msg{};
    mavlink_msg_camera_capture_status_pack(
        _server_component_impl->get_own_system_id(),
        _server_component_impl->get_own_component_id(),
        &msg,
        static_cast<uint32_t>(_server_component_impl->get_time().elapsed_s() * 1e3),
        image_status,
        video_status,
        _image_capture_timer_interval_s,
        recording_time_ms,
        available_capacity,
        _image_capture_count);

    _server_component_impl->send_message(msg);
    LogDebug() << "send capture status";
    return CameraServer::Result::Success;
}

CameraServer::FormatStorageHandle
CameraServerImpl::subscribe_format_storage(const CameraServer::FormatStorageCallback& callback)
{
    return _format_storage_callbacks.subscribe(callback);
}
void CameraServerImpl::unsubscribe_format_storage(CameraServer::FormatStorageHandle handle)
{
    _format_storage_callbacks.unsubscribe(handle);
}

CameraServer::ResetSettingsHandle
CameraServerImpl::subscribe_reset_settings(const CameraServer::ResetSettingsCallback& callback)
{
    return _reset_settings_callbacks.subscribe(callback);
}

void CameraServerImpl::unsubscribe_reset_settings(CameraServer::ResetSettingsHandle handle)
{
    _reset_settings_callbacks.unsubscribe(handle);
}

/**
 * Starts capturing images with the given interval.
 * @param [in]  interval_s      The interval between captures in seconds.
 * @param [in]  count           The number of images to capture or 0 for "forever".
 * @param [in]  index           The index/sequence number pass to the user callback.
 */
void CameraServerImpl::start_image_capture_interval(float interval_s, int32_t count, int32_t index)
{
    // If count == 0, it means capture "forever" until a stop command is received.
    auto remaining = std::make_shared<int32_t>(count == 0 ? INT32_MAX : count);
    auto take_photo_count = std::make_shared<int32_t>(0);

    _server_component_impl->add_call_every(
        [this, remaining, index, take_photo_count]() {
            LogDebug() << "capture image timer triggered";

            if (!_take_photo_callbacks.empty()) {
                _take_photo_callbacks(index + *take_photo_count);
                (*take_photo_count)++;
                (*remaining)--;
            }

            if (*remaining == 0) {
                stop_image_capture_interval();
            }
        },
        interval_s,
        &_image_capture_timer_cookie);

    _is_image_capture_interval_set = true;
    _image_capture_timer_interval_s = interval_s;
}

/**
 * Stops any pending image capture interval timer.
 */
void CameraServerImpl::stop_image_capture_interval()
{
    if (_image_capture_timer_cookie) {
        _server_component_impl->remove_call_every(_image_capture_timer_cookie);
    }

    _image_capture_timer_cookie = nullptr;
    _is_image_capture_interval_set = false;
    _image_capture_timer_interval_s = 0;
}

std::optional<mavlink_message_t> CameraServerImpl::process_camera_information_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto capabilities = static_cast<bool>(command.params.param1);

    if (!capabilities) {
        LogDebug() << "early info return";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    }

    if (!_is_information_set) {
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_TEMPORARILY_REJECTED);
    }

    // ack needs to be sent before camera information message
    auto ack_msg =
        _server_component_impl->make_command_ack_message(command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    _server_component_impl->send_message(ack_msg);

    // It is safe to ignore the return value of parse_version_string() here
    // since the string was already validated in set_information().
    uint32_t firmware_version;
    parse_version_string(_information.firmware_version, firmware_version);

    // capability flags are determined by subscriptions
    uint32_t capability_flags{};

    if (!_take_photo_callbacks.empty()) {
        capability_flags |= CAMERA_CAP_FLAGS::CAMERA_CAP_FLAGS_CAPTURE_IMAGE;
    }

    mavlink_message_t msg{};
    mavlink_msg_camera_information_pack(
        _server_component_impl->get_own_system_id(),
        _server_component_impl->get_own_component_id(),
        &msg,
        static_cast<uint32_t>(_server_component_impl->get_time().elapsed_s() * 1e3),
        reinterpret_cast<const uint8_t*>(_information.vendor_name.c_str()),
        reinterpret_cast<const uint8_t*>(_information.model_name.c_str()),
        firmware_version,
        _information.focal_length_mm,
        _information.horizontal_sensor_size_mm,
        _information.vertical_sensor_size_mm,
        _information.horizontal_resolution_px,
        _information.vertical_resolution_px,
        _information.lens_id,
        capability_flags,
        _information.definition_file_version,
        _information.definition_file_uri.c_str());

    _server_component_impl->send_message(msg);
    LogDebug() << "sent info msg";

    // ack was already sent
    return std::nullopt;
}

std::optional<mavlink_message_t> CameraServerImpl::process_camera_settings_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto settings = static_cast<bool>(command.params.param1);

    if (!settings) {
        LogDebug() << "early settings return";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    }

    // ack needs to be sent before camera information message
    auto ack_msg =
        _server_component_impl->make_command_ack_message(command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    _server_component_impl->send_message(ack_msg);

    // unsupported
    const auto mode_id = CAMERA_MODE::CAMERA_MODE_IMAGE;
    const float zoom_level = 0;
    const float focus_level = 0;

    mavlink_message_t msg{};
    mavlink_msg_camera_settings_pack(
        _server_component_impl->get_own_system_id(),
        _server_component_impl->get_own_component_id(),
        &msg,
        static_cast<uint32_t>(_server_component_impl->get_time().elapsed_s() * 1e3),
        mode_id,
        zoom_level,
        focus_level);

    _server_component_impl->send_message(msg);
    LogDebug() << "sent settings msg";

    // ack was already sent
    return std::nullopt;
}

std::optional<mavlink_message_t> CameraServerImpl::process_storage_information_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto storage_id = static_cast<uint8_t>(command.params.param1);
    auto information = static_cast<bool>(command.params.param2);

    if (!information) {
        LogDebug() << "early storage return";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    }

    if (_storage_information_callbacks.empty()) {
        LogDebug()
            << "Get storage information requested with no set storage information subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    // ack needs to be sent before storage information message
    auto ack_msg =
        _server_component_impl->make_command_ack_message(command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    _server_component_impl->send_message(ack_msg);

    _last_storage_id = storage_id;
    _storage_information_callbacks(storage_id);

    // result will return in respond_storage_information
    return std::nullopt;
}

std::optional<mavlink_message_t>
CameraServerImpl::process_storage_format(const MavlinkCommandReceiver::CommandLong& command)
{
    auto storage_id = static_cast<uint8_t>(command.params.param1);
    auto format = static_cast<bool>(command.params.param2);
    auto reset_image_log = static_cast<bool>(command.params.param3);

    UNUSED(format);
    UNUSED(reset_image_log);
    if (_format_storage_callbacks.empty()) {
        LogDebug() << "process storage format requested with no storage format subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _format_storage_callbacks(storage_id);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t> CameraServerImpl::process_camera_capture_status_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto capture_status = static_cast<bool>(command.params.param1);

    if (!capture_status) {
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    }

    if (_capture_status_callbacks.empty()) {
        LogDebug() << "process camera capture status requested with no capture status subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    // ack needs to be sent before camera capture status message
    auto ack_msg =
        _server_component_impl->make_command_ack_message(command, MAV_RESULT::MAV_RESULT_ACCEPTED);
    _server_component_impl->send_message(ack_msg);

    _capture_status_callbacks(0);

    // result will return in respond_capture_status
    return std::nullopt;
}

std::optional<mavlink_message_t>
CameraServerImpl::process_reset_camera_settings(const MavlinkCommandReceiver::CommandLong& command)
{
    auto reset = static_cast<bool>(command.params.param1);

    UNUSED(reset);

    if (_reset_settings_callbacks.empty()) {
        LogDebug() << "reset camera settings requested with no camera settings subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _reset_settings_callbacks(0);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_set_camera_mode(const MavlinkCommandReceiver::CommandLong& command)
{
    auto camera_mode = static_cast<CAMERA_MODE>(command.params.param2);

    if (_set_mode_callbacks.empty()) {
        LogDebug() << "Set mode requested with no set mode subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    // convert camera mode enum type
    CameraServer::Mode convert_camera_mode = CameraServer::Mode::Unknown;
    if (camera_mode == CAMERA_MODE_IMAGE) {
        convert_camera_mode = CameraServer::Mode::Photo;
    } else if (camera_mode == CAMERA_MODE_VIDEO) {
        convert_camera_mode = CameraServer::Mode::Video;
    }

    if (convert_camera_mode == CameraServer::Mode::Unknown) {
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }
    _set_mode_callbacks(convert_camera_mode);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_set_camera_zoom(const MavlinkCommandReceiver::CommandLong& command)
{
    auto zoom_type = static_cast<CAMERA_ZOOM_TYPE>(command.params.param1);
    auto zoom_value = command.params.param2;

    UNUSED(zoom_type);
    UNUSED(zoom_value);

    LogDebug() << "unsupported set camera zoom request";

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_set_camera_focus(const MavlinkCommandReceiver::CommandLong& command)
{
    auto focus_type = static_cast<SET_FOCUS_TYPE>(command.params.param1);
    auto focus_value = command.params.param2;

    UNUSED(focus_type);
    UNUSED(focus_value);

    LogDebug() << "unsupported set camera focus request";

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_set_storage_usage(const MavlinkCommandReceiver::CommandLong& command)
{
    auto storage_id = static_cast<uint8_t>(command.params.param1);
    auto usage = static_cast<STORAGE_USAGE_FLAG>(command.params.param2);

    UNUSED(storage_id);
    UNUSED(usage);

    LogDebug() << "unsupported set storage usage request";

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_image_start_capture(const MavlinkCommandReceiver::CommandLong& command)
{
    auto interval_s = command.params.param2;
    auto total_images = static_cast<int32_t>(command.params.param3);
    auto seq_number = static_cast<int32_t>(command.params.param4);

    LogDebug() << "received image start capture request - interval: " << +interval_s
               << " total: " << +total_images << " index: " << +seq_number;

    // TODO: validate parameters and return MAV_RESULT_DENIED not valid

    stop_image_capture_interval();

    if (_take_photo_callbacks.empty()) {
        LogDebug() << "image capture requested with no take photo subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    // single image capture
    if (total_images == 1) {
        if (seq_number < _image_capture_count) {
            LogDebug() << "received invalid single image capture request seq number : "
                       << seq_number << " image capture count " << _image_capture_count;
            // We know we already captured this request, so we can just ack it.
            return _server_component_impl->make_command_ack_message(
                command, MAV_RESULT::MAV_RESULT_ACCEPTED);
        }

        // MAV_RESULT_ACCEPTED must be sent before CAMERA_IMAGE_CAPTURED
        auto ack_msg = _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_ACCEPTED);
        _server_component_impl->send_message(ack_msg);

        _last_take_photo_command = command;

        _take_photo_callbacks(seq_number);

        return std::nullopt;
    }

    start_image_capture_interval(interval_s, total_images, seq_number);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_image_stop_capture(const MavlinkCommandReceiver::CommandLong& command)
{
    LogDebug() << "received image stop capture request";

    // REVISIT: should we return something other that MAV_RESULT_ACCEPTED if
    // there is not currently a capture interval active?
    stop_image_capture_interval();

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t> CameraServerImpl::process_camera_image_capture_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto seq_number = static_cast<uint32_t>(command.params.param1);

    UNUSED(seq_number);

    LogDebug() << "unsupported image capture request";

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_video_start_capture(const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);
    auto status_frequency = command.params.param2;

    UNUSED(status_frequency);

    if (_start_video_callbacks.empty()) {
        LogDebug() << "video start capture requested with no video start capture subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _start_video_callbacks(stream_id);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_video_stop_capture(const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);

    if (_stop_video_callbacks.empty()) {
        LogDebug() << "video stop capture requested with no video stop capture subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _stop_video_callbacks(stream_id);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_video_start_streaming(const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);

    if (_start_video_streaming_callbacks.empty()) {
        LogDebug() << "video start streaming requested with no video start streaming subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _start_video_streaming_callbacks(stream_id);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t>
CameraServerImpl::process_video_stop_streaming(const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);

    if (_stop_video_streaming_callbacks.empty()) {
        LogDebug() << "video stop streaming requested with no video stop streaming subscriber";
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
    }

    _stop_video_streaming_callbacks(stream_id);

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t> CameraServerImpl::process_video_stream_information_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);
    UNUSED(stream_id);

    if (!_is_video_stream_info_set) {
        return _server_component_impl->make_command_ack_message(
            command, MAV_RESULT::MAV_RESULT_TEMPORARILY_REJECTED);
    }

    // loop send video stream info
    for (auto& video_stream_info : _video_stream_infos) {
        uint16_t flags = 0;
        if (video_stream_info.status ==
            CameraServer::VideoStreamInfo::VideoStreamStatus::InProgress) {
            flags &= VIDEO_STREAM_STATUS_FLAGS_RUNNING;
        }
        if (video_stream_info.spectrum ==
            CameraServer::VideoStreamInfo::VideoStreamSpectrum::Infrared) {
            flags &= VIDEO_STREAM_STATUS_FLAGS_THERMAL;
        }

        mavlink_message_t msg{};
        mavlink_msg_video_stream_information_pack(
            _server_component_impl->get_own_system_id(),
            _server_component_impl->get_own_component_id(),
            &msg,
            video_stream_info.stream_id,
            1,
            0,
            flags,
            video_stream_info.settings.frame_rate_hz,
            video_stream_info.settings.horizontal_resolution_pix,
            video_stream_info.settings.vertical_resolution_pix,
            video_stream_info.settings.bit_rate_b_s,
            video_stream_info.settings.rotation_deg,
            video_stream_info.settings.horizontal_fov_deg,
            "",
            video_stream_info.settings.uri.c_str());

        _server_component_impl->send_message(msg);
    }

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_ACCEPTED);
}

std::optional<mavlink_message_t> CameraServerImpl::process_video_stream_status_request(
    const MavlinkCommandReceiver::CommandLong& command)
{
    auto stream_id = static_cast<uint8_t>(command.params.param1);

    UNUSED(stream_id);

    LogDebug() << "unsupported video stream status request";

    return _server_component_impl->make_command_ack_message(
        command, MAV_RESULT::MAV_RESULT_UNSUPPORTED);
}

} // namespace mavsdk
