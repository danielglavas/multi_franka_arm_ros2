# Test baseline

Which tests already fail on the base before our own changes. A failure listed here is inherited
from upstream; a failure **not** listed here was caused by us. Task P0-T6, brief in
`docs/project/briefs/P0-T6.md`.

## Recorded

- Date: 2026-11-21
- `ultrasound` at `504098d`. Code is unchanged from `upstream/humble` `44076f1`; only docs and Claude setup were added.
- Submodule `mujoco_ros_pkgs` at `b3a65f3`.
- Linter versions from apt (ROS 2 Humble, Ubuntu 22.04). cppcheck is 2.7.
- Commands:
  ```bash
  source ~/start_env.sh
  cd ~/ros2_ws_3
  colcon test --parallel-workers 2
  colcon test-result --all
  ```
- Run time: 17 s for all 28 packages.

## Totals

`colcon test-result` summary: **3268 tests, 0 errors, 2888 failures, 142 skipped.**

These numbers are only a rough guide, for two reasons:
- **Double counting:** each package appears twice, once as CTest's `Test.xml` (one entry per
  linter) and once per linter result file (one entry per file or per finding).
- **Granularity:** clang_format and uncrustify count every differing code chunk, so one reformatted file changes the total by dozens.

Compare against the **failing test names per package** below, not against the total.

## Failing tests per package

The CTest test name is what `colcon test` reports and what `ctest -R <name>` runs.

| Package | Failing CTest tests | Notes |
|---|---|---|
| `franka_bringup` | `copyright`, `flake8` | |
| `franka_control2` | `lint_cmake`, `uncrustify` | |
| `franka_description` | `flake8` | `franka_description_urdf_tests` fixed in P0-T4 (2026-11-21), now passes 6 of 6 |
| `franka_example_controllers` | `clang_format`, `copyright`, `lint_cmake` | |
| `franka_gripper` | `lint_cmake` | |
| `franka_hardware` | `clang_format`, `copyright`, `lint_cmake` | |
| `franka_moveit_config` | `flake8` | `franka_moveit_config_srdf_tests` passes (3 of 3) |
| `franka_semantic_components` | `clang_format`, `copyright` | |
| `franka_simple_publishers` (pytest) | `test_flake8` | `test_copyright` skipped |
| `garmi_bringup` | `copyright`, `flake8` | |
| `garmi_controllers` | `lint_cmake`, `uncrustify` | |
| `garmi_moveit_config` | `flake8` | |
| `mujoco_ros2_control` | `flake8`, `uncrustify` | |
| `mujoco_ros2_control_system` | `uncrustify` | |
| `mujoco_ros2_dummy_plugin` | `flake8`, `uncrustify` | |
| `mujoco_ros2_sensors` | `uncrustify` | |
| `multi_mode_controller` | `lint_cmake`, `uncrustify` | |
| `multi_mode_controller_impl` | `uncrustify` | |
| `panda_motion_generators` | `lint_cmake`, `uncrustify` | |
| `tf2_publisher` (pytest) | `test_flake8` | `test_copyright` skipped |

No failures in these packages:
- `franka_msgs`
- `franka_robot_state_broadcaster`
- `garmi_description`
- `multi_mode_control_msgs`
- `panda_motion_generator_msgs`

No tests at all:
- `mujoco_ros`
- `mujoco_ros_msgs`
- `mujoco_ros_pkgs`

## Causes

- **`franka_description_urdf_tests` was the only functional failure (fixed in P0-T4).** The test loaded
  `robots/panda_arm.urdf.xacro`. Upstream had moved the files: the arm is now in
  `robots/real/panda_arm.urdf.xacro` and `robots/sim/panda_arm_sim.urdf.xacro`, and the shared
  parts are in `robots/common/`. All 6 cases stopped with "No such file or directory". On
  2026-11-21 the path was changed to `robots/real/`, and all 6 pass. That was the only change from
  the recorded run, so the rest of the baseline is still valid without recording it again.
- **Style only, nothing wrong with the behaviour:**
  - `flake8`: long lines (E501), trailing whitespace (W291, W293), unused imports (F401).
  - `clang_format` and `uncrustify`: C++ formatting differs from the configured style.
  - `lint_cmake`: mostly "Line ends in whitespace" in `CMakeLists.txt`.
  - `copyright`: "could not find copyright notice" in some files.
- **cppcheck is skipped everywhere (142 skips).** `ament_cppcheck` skips cppcheck 2.x as too slow
  unless `AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS` is set. So no static C++ analysis runs at the moment.

## What "tests pass" means from now on

A change passes the tests when both of these hold:
1. `colcon test-result` shows no failing CTest test (or pytest test, for the Python packages) that is missing from the table above.
2. Every package under `ultrasound/` has **zero** failures, with no exceptions and no baseline.

A failure in the table that disappears is fine; update the table in the same commit. A change
inside a `franka_*` package must not raise that linter's failure count in the file it touches.
Check it with `colcon test-result --all --verbose` before and after.

To check only the packages you changed:
```bash
colcon test --packages-select <pkg>
colcon test-result --verbose --test-result-base build/<pkg>
```
Without `--test-result-base`, `colcon test-result` reads the whole `build/` tree and also reports the
old `franka_*` failures.

## Our packages (`ultrasound/`)

Added in P0-T7 (2026-11-23). All pass with zero failures and zero skips (40 tests in total).
`ultrasound_scan_execution` is the only package where cppcheck really runs: its `CMakeLists.txt`
sets `AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1` for that one test. Everywhere else cppcheck stays skipped.

## When to record the baseline again

- After rebasing onto a newer `upstream/humble`, or after updating the `mujoco_ros_pkgs` submodule.
- After an apt upgrade that changes the ament linter or cppcheck versions.
