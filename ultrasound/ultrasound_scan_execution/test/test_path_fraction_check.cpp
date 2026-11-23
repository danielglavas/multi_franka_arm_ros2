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

// Unit tests for is_path_fraction_acceptable.

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include "ultrasound_scan_execution/path_fraction_check.hpp"

using ultrasound_scan_execution::is_path_fraction_acceptable;

TEST(PathFractionCheck, AchievedAboveMinimumIsAcceptable)
{
  EXPECT_TRUE(is_path_fraction_acceptable(0.95, 0.90));
}

TEST(PathFractionCheck, AchievedBelowMinimumIsNotAcceptable)
{
  EXPECT_FALSE(is_path_fraction_acceptable(0.85, 0.90));
}

TEST(PathFractionCheck, EqualValuesAreAcceptable)
{
  EXPECT_TRUE(is_path_fraction_acceptable(1.0, 1.0));
  EXPECT_TRUE(is_path_fraction_acceptable(0.0, 0.0));
}

TEST(PathFractionCheck, AchievedOutOfRangeThrows)
{
  EXPECT_THROW(is_path_fraction_acceptable(-0.1, 0.5), std::invalid_argument);
  EXPECT_THROW(is_path_fraction_acceptable(1.1, 0.5), std::invalid_argument);
}

TEST(PathFractionCheck, MinimumOutOfRangeThrows)
{
  EXPECT_THROW(is_path_fraction_acceptable(0.5, -0.1), std::invalid_argument);
  EXPECT_THROW(is_path_fraction_acceptable(0.5, 1.1), std::invalid_argument);
}

TEST(PathFractionCheck, NanThrows)
{
  const double not_a_number = std::numeric_limits<double>::quiet_NaN();
  EXPECT_THROW(is_path_fraction_acceptable(not_a_number, 0.5), std::invalid_argument);
  EXPECT_THROW(is_path_fraction_acceptable(0.5, not_a_number), std::invalid_argument);
}
