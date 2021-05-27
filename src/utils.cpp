/* Copyright 2020 Dandem Labs
 * Authored by Michael de Gans <mike@mdegans.dev>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "utils.h"

#include <iomanip>
#include <regex>
#include <string>

using std::experimental::optional;

namespace ptzf {

/**
 * cstring to listen for when go destination is reached.
 */
const char OK_CODE[] = "ok P63 B31";

/**
 * code to indicate printer is busy
 */
const char BUSY_CODE[] = "echo:busy:";

/**
 * cstring to listen for when error occurs.
 */
const char ERROR_CODE[] = "error";

/**
 * Regex for a position line (M114 return)
 */
const std::regex RX_LINE(
    R"ESC(X:(\d{1,2}\.\d{2}) Y:(\d{1,2}\.\d{2}) Z:(\d{1,2}\.\d{2}) E:(\d{1,2}\.\d{2}) Count X)ESC");

void join_and_log(std::thread& t) {
  if (t.joinable()) {
    LOG(debug) << "Joining " << t.get_id();
    t.join();
  }
}

optional<Position> string_to_position(const std::string& line) {
  LOG(debug) << "Parsing:" << line;
  std::smatch match;
  if (!std::regex_match(line, match, RX_LINE)) {
    // some random line, possibly an "ok ..."
    // todo: forward any errors to boost log
    return {};
  }
  try {
    // parse line here and return a Position if possible
    return Position{std::stof(match[0]), std::stof(match[1]),
                    std::stof(match[2]), std::stof(match[3])};
  } catch (const std::exception& e) {
    // this could happen if the matched values somehow could not convert to
    // float (this should never happen unless the regex is broken).
    LOG(error) << "Could not parse:" << line << " because " << e.what();
  }
  return {};
}

std::string position_to_string(const Position& p) {
  std::stringstream s;
  s << std::fixed << std::setprecision(3) << "G0 X" << p.x << " Y" << p.y
    << " Z" << p.z << " E" << p.f;
  return s.str();
}

bool wait_for_ok(LibSerial::SerialStream& stream) {
  if (!stream.IsOpen()) {
    LOG(error) << "Stream not open. Can't wait for \"OK\"";
    return false;
  }

  LOG(debug) << "Waiting for \"OK\"";
  std::string line;
  while (stream.IsOpen()) {
    std::getline(stream, line);
    LOG(debug) << "recv:" << line;
    // if line starts with OK_CODE
    if (!line.compare(0, sizeof(OK_CODE), OK_CODE)) {
      LOG(debug) << "Found OK_CODE in line: \"" << line << '"';
      return true;
    };
    // if line starts with BUSY_CODE
    if (!line.compare(0, sizeof(BUSY_CODE), BUSY_CODE)) {
      LOG(debug) << "Found BUSY_CODE in line: \"" << line << '"';
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    };
    // if line starts with ERROR_CODE.
    if (!line.compare(0, sizeof(ERROR_CODE), ERROR_CODE)) {
      LOG(error) << line;
      return false;
    };
  }

  LOG(error) << "reached end of context somehow";
  return false;
}

}  // namespace ptzf
