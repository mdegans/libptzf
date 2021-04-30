#ifndef C083A083_8E43_4310_9FAA_11E0CEB64D65
#define C083A083_8E43_4310_9FAA_11E0CEB64D65

#include <experimental/optional>
#include <thread>

#include <boost/log/trivial.hpp>

#include <SerialStream.h>

#include "position.h"

// our logging macro
#define LOG BOOST_LOG_TRIVIAL

namespace ptzf {

/**
 * Joins a thread and LOG to debug level.
 */
void join_and_log(std::thread& t);

/**
 * Parse a line of GCode and return a Position if possible
 */
std::experimental::optional<Position> string_to_position(const std::string& line);

/**
 * Convert a position to string.
 */
std::string position_to_string(const Position& p);

/**
 * @brief Wait for an OK signal
 * 
 * @param stream stream to wait on.
 * 
 * @return true on success
 * @return false on error
 */
bool wait_for_ok(LibSerial::SerialStream& stream);

}  // namespace ptzf

#endif /* C083A083_8E43_4310_9FAA_11E0CEB64D65 */
