#pragma once
#include <unordered_map>

#include "plugins/param_server/param_server.h"
#include "server_plugin_impl_base.h"
#include "mavlink_parameter_server.h"
#include "callback_list.h"

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

    ParamServer::Result
    provide_param_custom(std::string name, std::string value, ParamServer::Type value_type) const;

    ParamServer::AllParams retrieve_all_params() const;

    ParamServer::ParamChangedHandle subscribe_param_changed(
        std::string name, ParamServer::Type type, const ParamServer::ParamChangedCallback callback);
    void unsubscribe_param_changed(ParamServer::ParamChangedHandle handle);

    static ParamServer::Result
    result_from_mavlink_parameter_server_result(MavlinkParameterServer::Result result);

private:
    CallbackList<std::string> _param_changed_callback{};
    std::unordered_map<std::string, ParamServer::ParamChangedCallback> _param_changed_callback_map;
};

} // namespace mavsdk
