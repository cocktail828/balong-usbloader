#include "log.h"

namespace LOGGER
{
std::ofstream Logger::fout;
Severity Logger::level_basic = Severity::INFO;
FileMode Logger::mode = FileMode::OVERWRITE;
Logger LOG;
}; // namespace LOGGER