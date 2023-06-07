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

    ParamServer::Result provide_param_custom(std::string name, std::string value) const;

    ParamServer::AllParams retrieve_all_params() const;

    ParamServer::CustomParamChangedHandle
    subscribe_custom_param_changed(const ParamServer::CustomParamChangedCallback callback);
    void unsubscribe_custom_param_changed(ParamServer::CustomParamChangedHandle handle);

    static ParamServer::Result
    result_from_mavlink_parameter_server_result(MavlinkParameterServer::Result result);

private:
    mutable CallbackList<std::string> _custom_param_changed_callbacks{};
    mutable ParamServer::CustomParamChangedCallback _custom_param_changed_callback;
};

} // namespace mavsdk
