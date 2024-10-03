#include "utils.hxx"
#include <cstdlib>

const char *Utils::GetEnv(const char *tag) noexcept
{
    const char *ret = std::getenv(tag);
    return ret ? ret : nullptr;
}