#include <log/log.hpp>

#include <iostream>
#include <string_view>

// Provide a log handler
namespace custom {
struct log_handler {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename FmtResult>
    auto log(FilenameStringType f, LineNumberType n, FmtResult const &fr)
        -> void {
        std::cout << "Log: " << f << ":" << n << ": "
                  << std::string_view{fr.str.value} << '\n';
    }
};

// Provide a log config with that handler
struct config {
    log_handler logger;
};
} // namespace custom

// specialize the logging config variable template to use the
// custom config
template <> inline auto logging::config<> = custom::config{};

// logs will now go to the custom logger
auto main() -> int { CIB_INFO("Hello"); }
