#include "param_server_impl.h"

namespace mavsdk {

ParamServerImpl::ParamServerImpl(std::shared_ptr<ServerComponent> server_component) :
    ServerPluginImplBase(server_component)
{
    // FIXME: this plugin should support various component IDs
    _server_component_impl->register_plugin(this);
}

ParamServerImpl::~ParamServerImpl()
{
    _server_component_impl->unregister_plugin(this);
}

void ParamServerImpl::init() {}

void ParamServerImpl::deinit() {}

std::pair<ParamServer::Result, int32_t> ParamServerImpl::retrieve_param_int(std::string name) const
{
    auto result =
        _server_component_impl->mavlink_parameter_server().retrieve_server_param_int(name);

    if (result.first == MavlinkParameterServer::Result::Success) {
        return {ParamServer::Result::Success, result.second};
    } else {
        return {ParamServer::Result::NotFound, -1};
    }
}

ParamServer::Result ParamServerImpl::provide_param_int(std::string name, int32_t value)
{
    if (name.size() > 16) {
        return ParamServer::Result::ParamNameTooLong;
    }
    _server_component_impl->mavlink_parameter_server().provide_server_param_int(name, value);
    if (_custom_param_changed_callback != nullptr) {
        _server_component_impl->mavlink_parameter_server().subscribe_param_int_changed(
            name,
            [this, name](int value) {
                ParamServer::CustomParam custom_param;
                custom_param.name = name;
                custom_param.value = std::to_string(value);
                _custom_param_changed_callback(custom_param);
            },
            nullptr);
    }
    return ParamServer::Result::Success;
}

std::pair<ParamServer::Result, float> ParamServerImpl::retrieve_param_float(std::string name) const
{
    const auto result =
        _server_component_impl->mavlink_parameter_server().retrieve_server_param_float(name);

    if (result.first == MavlinkParameterServer::Result::Success) {
        return {ParamServer::Result::Success, result.second};
    } else {
        return {ParamServer::Result::NotFound, NAN};
    }
}

ParamServer::Result ParamServerImpl::provide_param_float(std::string name, float value)
{
    if (name.size() > 16) {
        return ParamServer::Result::ParamNameTooLong;
    }
    _server_component_impl->mavlink_parameter_server().provide_server_param_float(name, value);
    if (_custom_param_changed_callback != nullptr) {
        _server_component_impl->mavlink_parameter_server().subscribe_param_float_changed(
            name,
            [this, name](float value) {
                ParamServer::CustomParam custom_param;
                custom_param.name = name;
                custom_param.value = std::to_string(value);
                _custom_param_changed_callback(custom_param);
            },
            nullptr);
    }
    return ParamServer::Result::Success;
}

std::pair<ParamServer::Result, std::string>
ParamServerImpl::retrieve_param_custom(std::string name) const
{
    const auto result =
        _server_component_impl->mavlink_parameter_server().retrieve_server_param_custom(name);

    if (result.first == MavlinkParameterServer::Result::Success) {
        return {ParamServer::Result::Success, result.second};
    } else {
        return {ParamServer::Result::NotFound, {}};
    }
}

ParamServer::Result ParamServerImpl::provide_param_custom(std::string name, std::string value) const
{
    if (name.size() > 16) {
        return ParamServer::Result::ParamNameTooLong;
    }

    _server_component_impl->mavlink_parameter_server().provide_server_param_custom(name, value);
    if (_custom_param_changed_callback != nullptr) {
        _server_component_impl->mavlink_parameter_server().subscribe_param_changed<std::string>(
            name,
            [this, name](std::string value) {
                ParamServer::CustomParam custom_param;
                custom_param.name = name;
                custom_param.value = value;
                _custom_param_changed_callback(custom_param);
            },
            nullptr);
    }
    return ParamServer::Result::Success;
}

ParamServer::AllParams ParamServerImpl::retrieve_all_params() const
{
    auto tmp = _server_component_impl->mavlink_parameter_server().retrieve_all_server_params();

    ParamServer::AllParams res{};

    for (auto const& param_pair : tmp) {
        if (param_pair.second.is<float>()) {
            ParamServer::FloatParam tmp_param;
            tmp_param.name = param_pair.first;
            tmp_param.value = param_pair.second.get<float>();
            res.float_params.push_back(tmp_param);
        } else if (param_pair.second.is<int32_t>()) {
            ParamServer::IntParam tmp_param;
            tmp_param.name = param_pair.first;
            tmp_param.value = param_pair.second.get<int32_t>();
            res.int_params.push_back(tmp_param);
        } else if (param_pair.second.is<std::string>()) {
            ParamServer::CustomParam tmp_param;
            tmp_param.name = param_pair.first;
            tmp_param.value = param_pair.second.get<int32_t>();
            res.custom_params.push_back(tmp_param);
        }
    }

    return res;
}

ParamServer::CustomParamChangedHandle ParamServerImpl::subscribe_custom_param_changed(
    const ParamServer::CustomParamChangedCallback callback)
{
    _custom_param_changed_callback = callback;
    // return _custom_param_changed_callbacks.subscribe(callback);
}

void ParamServerImpl::unsubscribe_custom_param_changed(ParamServer::CustomParamChangedHandle handle)
{
    _custom_param_changed_callback = nullptr;
    //_custom_param_changed_callbacks.unsubscribe(handle);
}

ParamServer::Result
ParamServerImpl::result_from_mavlink_parameter_server_result(MavlinkParameterServer::Result result)
{
    switch (result) {
        case MavlinkParameterServer::Result::Success:
            return ParamServer::Result::Success;
        case MavlinkParameterServer::Result::NotFound:
            return ParamServer::Result::NotFound;
        case MavlinkParameterServer::Result::ParamNameTooLong:
            return ParamServer::Result::ParamNameTooLong;
        case MavlinkParameterServer::Result::WrongType:
            return ParamServer::Result::WrongType;
        case MavlinkParameterServer::Result::ParamValueTooLong:
            return ParamServer::Result::ParamValueTooLong;
        default:
            LogErr() << "Unknown param error";
            return ParamServer::Result::Unknown;
    }
}

} // namespace mavsdk
