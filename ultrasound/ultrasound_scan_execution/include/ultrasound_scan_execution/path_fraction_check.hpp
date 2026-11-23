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

// Declares the check that decides whether MoveIt2 planned enough of a requested Cartesian path.

#ifndef ULTRASOUND_SCAN_EXECUTION__PATH_FRACTION_CHECK_HPP_
#define ULTRASOUND_SCAN_EXECUTION__PATH_FRACTION_CHECK_HPP_

namespace ultrasound_scan_execution
{

/// True when MoveIt2's achieved Cartesian path fraction is at least the required minimum.
/// Both values are fractions of the requested path, unitless, in [0.0, 1.0].
/// Throws std::invalid_argument (message names the bad argument and its value)
/// if either value is NaN or outside [0.0, 1.0].
bool is_path_fraction_acceptable(double achieved_fraction, double minimum_fraction);

}  // namespace ultrasound_scan_execution

#endif  // ULTRASOUND_SCAN_EXECUTION__PATH_FRACTION_CHECK_HPP_
