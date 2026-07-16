#pragma once

#include <string>
#include <array>

namespace RS::Containers {

    inline std::array<int, 3> version() noexcept {
        return {{ 0, 1, 0 }};
    }

    inline std::string version_string() {
        return "0.1.0";
    }

}
