#include "print_callbacks.hpp"


void btx_initialize_usr_data(common_data_t *common_data, void **usr_data) {
    /* User allocates its own data structure */
    struct tally_dispatch *data = new struct tally_dispatch;
    /* User makes our API usr_data to point to his/her data structure */
    *usr_data = data;

    params_t *params = (params_t*) malloc(sizeof(params_t));
    btx_read_params(common_data, params);

    data->display_compact = params->display_compact;
    data->display_human = params->display_human;
    data->display_metadata = params->display_metadata;
    data->display_name_max_size = params->display_name_max_size;

    free(params);
}

void btx_finalize_usr_data(common_data_t *common_data, void *usr_data) {
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;
    
    const int max_name_size = data->display_name_max_size;

    if (data->display_human) {
        if (data->display_metadata)
            print_metadata(data->metadata);

        if (data->display_compact) {

            for (const auto& [level,host]: data->host) {
                std::string s = join_iterator(data->host_backend_name[level]);
                print_compact(s, host,
                            std::make_tuple("Hostnames", "Processes", "Threads"),
                            max_name_size);
            }
            print_compact("Device profiling", data->device,
                            std::make_tuple("Hostnames", "Processes", "Threads",
                                            "Devices", "Subdevices"),
                            max_name_size);

            for (const auto& [level,traffic]: data->traffic) {
                std::string s = join_iterator(data->traffic_backend_name[level]);
                print_compact("Explicit memory traffic (" + s + ")", traffic,
                            std::make_tuple("Hostnames", "Processes", "Threads"),
                            max_name_size);
            }
        }else {
            for (const auto& [level,host]: data->host) {
                std::string s = join_iterator(data->host_backend_name[level]); 
                print_extended(s, host,
                            std::make_tuple("Hostname", "Process", "Thread"),
                            max_name_size);
            }
            print_extended("Device profiling", data->device,
                            std::make_tuple("Hostname", "Process", "Thread",
                                            "Device pointer", "Subdevice pointer"),
                            max_name_size);

            for (const auto& [level,traffic]: data->traffic) {
                std::string s = join_iterator(data->traffic_backend_name[level]);
                print_extended("Explicit memory traffic (" + s + ")", traffic,
                            std::make_tuple("Hostname", "Process", "Thread"),
                            max_name_size);
            }
        }
    } else {
        
        nlohmann::json j;
        j["units"] = {{"time", "ns"}, {"size", "bytes"}};
        
        if (data->display_metadata)
            j["metadata"] = data->metadata;
        
        if (data->display_compact) {
            for (auto& [level,host]: data->host)
                j["host"][level] = json_compact(host);

            if (!data->device.empty())
                j["device"] = json_compact(data->device);

            for (auto& [level,traffic]: data->traffic)
                j["traffic"][level] = json_compact(traffic);

        } else {
            for (auto& [level,host]: data->host)
                j["host"][level] = json_extented(host, std::make_tuple("Hostname", "Process", "Thread"));
                
            if (!data->device.empty())
                j["device"] = json_extented(data->device,std::make_tuple(
                    "Hostname", "Process","Thread", "Device pointer","Subdevice pointer"));

            for (auto& [level,traffic]: data->traffic)
                    j["traffic"][level] = json_extented(traffic,std::make_tuple(
                        "Hostname", "Process", "Thread"));
        }
        std::cout << j << std::endl;
    }

    /* Desolate user data */
    delete data;
}

void tally_host_usr_callbacks(
    common_data_t *common_data, void *usr_data, const char* hostname, int64_t vpid, 
    uint64_t vtid, int64_t backend, const char* name, uint64_t dur, uint64_t min, 
    uint64_t max, uint64_t count, uint64_t err
)
{
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;

    TallyCoreTime a{dur, err};
    const int level = backend_level[backend];
    data->host_backend_name[level].insert(backend_name[backend]);
    data->host[level][hpt_function_name_t(hostname, vpid, vtid, name)] += a;
}

void tally_device_usr_callbacks(
    common_data_t *common_data, void *usr_data, const char* hostname, int64_t vpid,
    uint64_t vtid, int64_t backend, const char* name, uint64_t did, uint64_t sdid,
    uint64_t dur, uint64_t min, uint64_t max, uint64_t count, uint64_t err
)
{
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;

    TallyCoreTime a{dur, err};
    data->device[hpt_device_function_name_t(hostname, vpid, vtid, did, sdid, name)] += a;
}

void tally_traffic_usr_callbacks(
    common_data_t *common_data, void *usr_data, const char* hostname, int64_t vpid,
    uint64_t vtid, int64_t backend, const char* name, uint64_t size, uint64_t min,
    uint64_t max, uint64_t count, uint64_t err
)
{
    /* In callbacks, the user just  need to cast our API usr_data to his/her data structure */
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;

    TallyCoreByte a{(uint64_t)size, false};
    const int level = backend_level[backend];
    data->traffic_backend_name[level].insert(backend_name[backend]);
    data->traffic[level][hpt_function_name_t(hostname, vpid, vtid, name)] += a;
}

void lttng_device_name_usr_callbacks(
    common_data_t *common_data, void *usr_data, const char* hostname, int64_t vpid,
    uint64_t vtid, int64_t backend, const char* name, uint64_t did
)
{
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;
    data->device_name[hp_device_t(hostname, vpid, did)] = name;
}

void lttng_ust_thapi_metadata_usr_callbacks(
    common_data_t *common_data, void *usr_data, const char* hostname, int64_t vpid,
    uint64_t vtid, int64_t backend, const char* metadata
)
{
    struct tally_dispatch *data = (struct tally_dispatch *) usr_data;
    data->metadata.push_back(metadata);
} 

void btx_register_usr_callbacks(name_to_dispatcher_t** name_to_dispatcher) {
    btx_register_callbacks_tally_host(name_to_dispatcher,&tally_host_usr_callbacks);
    btx_register_callbacks_tally_device(name_to_dispatcher,&tally_device_usr_callbacks);
    btx_register_callbacks_tally_traffic(name_to_dispatcher,&tally_traffic_usr_callbacks);
    btx_register_callbacks_lttng_device_name(name_to_dispatcher,&lttng_device_name_usr_callbacks);
    btx_register_callbacks_lttng_ust_thapi_metadata(name_to_dispatcher,&lttng_ust_thapi_metadata_usr_callbacks);
}
