// Copyright 2026 Daniel Glavas
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Implements the check that decides whether MoveIt2 planned enough of a requested Cartesian path.

#include "ultrasound_scan_execution/path_fraction_check.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace ultrasound_scan_execution
{

namespace
{

/// Throws std::invalid_argument if the fraction is NaN or outside [0.0, 1.0].
/// The message names the argument and its value.
void require_valid_fraction(double fraction, const std::string & argument_name)
{
  // NaN compares false with everything, so it has to be tested on its own.
  if (std::isnan(fraction) || fraction < 0.0 || fraction > 1.0) {
    throw std::invalid_argument(
            argument_name + " must be a number in [0.0, 1.0], but is " + std::to_string(fraction));
  }
}

}  // namespace

bool is_path_fraction_acceptable(double achieved_fraction, double minimum_fraction)
{
  require_valid_fraction(achieved_fraction, "achieved_fraction");
  require_valid_fraction(minimum_fraction, "minimum_fraction");
  return achieved_fraction >= minimum_fraction;
}

}  // namespace ultrasound_scan_execution
