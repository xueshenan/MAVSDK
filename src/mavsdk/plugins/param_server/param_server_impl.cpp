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

ParamServer::Result ParamServerImpl::provide_param_custom(
    std::string name, std::string value, ParamServer::Type value_type) const
{
    if (name.size() > 16) {
        return ParamServer::Result::ParamNameTooLong;
    }

    // TODO: don't want to change the core compont so use the xml format
    std::string xml_type_string = "";
    switch (value_type) {
        case ParamServer::Type::Uint8:
            xml_type_string = "uint8";
            break;
        case ParamServer::Type::Int8:
            xml_type_string = "int8";
            break;
        case ParamServer::Type::Uint16:
            xml_type_string = "uint16";
            break;
        case ParamServer::Type::Int16:
            xml_type_string = "int16";
            break;
        case ParamServer::Type::Uint32:
            xml_type_string = "uint32";
            break;
        case ParamServer::Type::Int32:
            xml_type_string = "int32";
            break;
        case ParamServer::Type::Uint64:
            xml_type_string = "uint64";
            break;
        case ParamServer::Type::Int64:
            xml_type_string = "int64";
            break;
        case ParamServer::Type::Float:
            xml_type_string = "float";
            break;
        case ParamServer::Type::Double:
            xml_type_string = "double";
            break;
        default:
            break;
    }

    if (xml_type_string.empty()) {
        _server_component_impl->mavlink_parameter_server().provide_server_param_custom(name, value);
    } else {
        ParamValue param_value;
        param_value.set_from_xml(xml_type_string, value);
        _server_component_impl->mavlink_parameter_server().provide_server_param(name, param_value);
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
        }
    }

    return res;
}

ParamServer::ParamChangedHandle ParamServerImpl::subscribe_param_changed(
    std::string name, ParamServer::Type type, const ParamServer::ParamChangedCallback callback)
{
    _param_changed_callback_map[name] = callback;
    std::string c_name = name;
    // TODO: ugly code, need change
    switch (type) {
        case ParamServer::Type::Uint8:
        case ParamServer::Type::Int8:
        case ParamServer::Type::Uint16:
        case ParamServer::Type::Int16:
        case ParamServer::Type::Uint32:
        case ParamServer::Type::Int32:
        case ParamServer::Type::Uint64:
        case ParamServer::Type::Int64:
            _server_component_impl->mavlink_parameter_server().subscribe_param_changed<int>(
                name,
                [this, c_name](int value) {
                    _param_changed_callback_map[c_name](std::to_string(value));
                },
                nullptr);
            break;
        case ParamServer::Type::Float:
        case ParamServer::Type::Double:
            _server_component_impl->mavlink_parameter_server().subscribe_param_changed<float>(
                name,
                [this, c_name](float value) {
                    _param_changed_callback_map[c_name](std::to_string(value));
                },
                nullptr);
            break;
        case ParamServer::Type::String:
            _server_component_impl->mavlink_parameter_server().subscribe_param_changed<std::string>(
                name,
                [this, c_name](std::string value) { _param_changed_callback_map[c_name](value); },
                nullptr);
            break;
        case ParamServer::Type::Custom:
            LogErr() << "Unsupport custom value type";
            break;
    }
}

void ParamServerImpl::unsubscribe_param_changed(ParamServer::ParamChangedHandle handle) {}

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
