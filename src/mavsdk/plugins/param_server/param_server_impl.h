#pragma once
#include <unordered_map>

#include "plugins/param_server/param_server.h"
#include "server_plugin_impl_base.h"
#include "mavlink_parameter_server.h"

namespace mavsdk {

class ParamServerImpl : public ServerPluginImplBase {
public:
    explicit ParamServerImpl(std::shared_ptr<ServerComponent> server_component);
    ~ParamServerImpl() override;

    void init() override;
    void deinit() override;

    std::pair<ParamServer::Result, int32_t> retrieve_param_int(std::string name) const;

    ParamServer::Result provide_param_int(std::string name, int32_t value);

    std::pair<ParamServer::Result, float> retrieve_param_float(std::string name) const;

    ParamServer::Result provide_param_float(std::string name, float value);

    std::pair<ParamServer::Result, std::string> retrieve_param_custom(std::string name) const;

    ParamServer::Result provide_param_custom(
        std::string name, std::string value, ParamServer::ValueType value_type) const;

    ParamServer::AllParams retrieve_all_params() const;

    void subscribe_param_changed_async(
        std::string name,
        ParamServer::ValueType type,
        const ParamServer::SubscribeParamChangedCallback callback);

    static ParamServer::Result
    result_from_mavlink_parameter_server_result(MavlinkParameterServer::Result result);

private:
    std::unordered_map<std::string, ParamServer::SubscribeParamChangedCallback>
        _param_changed_callback_map;
};

} // namespace mavsdk
