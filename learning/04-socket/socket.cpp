#include "socket_util.hpp"

#include <print>
#include <vector>

int main()
{
    common::Logger logger("build-linux/app.log");
    std::vector<std::string> iface_names {"lo", "eth0"};
    for (const auto& name: iface_names) {
        std::string timestr;
        // logger.log("INFO %:% %() % " + std::string("ip of {}: {}") + "\n", __FILE__, __LINE__, __FUNCTION__, 
        //     common::get_now_str(&timestr), name, common::get_iface_ip(name));
        // logger.info("ip of {}: {}", name, common::get_iface_ip(name));
        std::println("ip of {}: {}", name, common::get_iface_ip(name));
    }

    return 0;
}