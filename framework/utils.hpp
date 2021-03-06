#ifndef __utils_included_034967102602157602372031257032075301265983470946067051
#define __utils_included_034967102602157602372031257032075301265983470946067051

#include <cstdint>

namespace utils {

//=======================================================================================================================================================================================================================
// read the whole file content in a char buffer
//=======================================================================================================================================================================================================================
char* file_read(const char* file_name);


namespace timer {

//=======================================================================================================================================================================================================================
// Cross-platform high resolution timer function.
//=======================================================================================================================================================================================================================

uint64_t ns();

} // namespace time

} // namespace utils

#endif // __utils_included_034967102602157602372031257032075301265983470946067051