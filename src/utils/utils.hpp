#pragma once
#include <wups.h>

#include <coreinit/title.h>

#include <cstring>
#include <string>
#include <vector>

namespace utils {
    bool isVinoClientID(const char *client_id);
    bool isVinoTitleID(uint32_t title_id);
} // namespace utils