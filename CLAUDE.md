# Panda + ultrasound probe + 3D Slicer

## Working with Daniel (student; Claude is coder and mentor)
- Before doing something, explain what, why and how.
- When making a choice, briefly name the alternatives and why this one won.
- All code gets comments explaining what it does. Names (variables, functions, topics) describe their purpose.
- Investigate and propose first; change files only after approval.

## System
Goal chain: Franka Emika Panda -> MoveIt2 -> ROS 2 Humble -> SlicerROS2 module -> 3D Slicer.
The end effector will be an ultrasound probe (not modelled yet; `hand.xacro` gripper is still the only EOAT).
- `franka_bringup franka_sim.launch.py` runs the Panda in MuJoCo (not fake hardware, not the real robot).
- SlicerROS2 makes Slicer a ROS 2 node: it reads the URDF from the `robot_description` parameter of
  `robot_state_publisher` and moves the links via tf2. It only displays STL/OBJ meshes.
- Slicer uses its own Python 3.9, so `rclpy` cannot be imported inside Slicer; use the module's MRML classes.
- MoveIt2 (apt, 2.5.10) is only partly installed and `franka_moveit_config` has no MuJoCo launch.

## Where things live
- This repo (`~/ros2_ws_2/src/multi_franka_arm_ros2`): main repo. All project work happens here.
- `~/ros2_ws/src/slicer_ros2_module`: SlicerROS2, separate workspace, kept separate on purpose.
  It has an uncommitted Humble workaround in `MRML/CMakeLists.txt`; never discard it.
- `~/slicer/Slicer`: Slicer 5.6.2 source. `~/slicer/Slicer-SuperBuild-Release/Slicer-build`: build tree
  with the `Slicer` launcher. Very large: search only targeted subfolders.
- The two workspaces do not depend on each other; each chains only `/opt/ros/humble`.

## Sourcing and running
- Claude's shell has no ROS environment. Prefix every `ros2`/`colcon` command with
  `source /opt/ros/humble/setup.bash && source /home/daniel/ros2_ws_2/install/setup.bash &&`.
- Terminal 1: source as above, then `ros2 launch franka_bringup franka_sim.launch.py`.
- Terminal 2 (after the sim is up): source as above, `cd ~/slicer/Slicer-SuperBuild-Release/Slicer-build && ./Slicer`,
  then add the robot in the ROS2 module.
- `~/ros2_ws/install/setup.bash` is only needed to build or test the Slicer module, not to run Slicer with the Panda.

## Build and test
- Build: `cd ~/ros2_ws_2 && colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release` (add `--packages-select <pkg>`).
- Not a symlink install: rebuild after editing xacro, launch, config or mesh files.
- Test: `colcon test --packages-select <pkg> && colcon test-result --verbose`.
- Existing tests: ament linters, plus pytest in `franka_description` and `franka_moveit_config`.
  gtest, gmock and launch_testing are installed but unused so far.
- Slicer module: `cd ~/ros2_ws && colcon build --cmake-args -DSlicer_DIR:PATH=/home/daniel/slicer/Slicer-SuperBuild-Release/Slicer-build -DCMAKE_BUILD_TYPE=Release`.
  This installs into the Slicer build tree, so ask before building it.
  Its tests run inside Slicer: `slicer.util.getModuleLogic('ROS2Tests').run()`.

## Safety
- Never start anything that can reach the real robot without asking: everything under
  `franka_bringup/launch/real` and `launch/mixed`, `franka_moveit_config` launches, `franka_gripper`,
  and `ros2 service call` / `topic pub` / `action send_goal` / controller switching.
- Never use `sudo`; give Daniel the command to run instead.

## Git (this repo)
- Work only on `ultrasound`. Never commit to `humble`; it mirrors `upstream/humble`.
- `origin` = Daniel's fork, `upstream` = original. Never push to `upstream`.
- Never force-push without asking. Commit and push only when asked.
- The fork on GitHub is public. Before every commit and push, check the staged changes for
  passwords, tokens, API keys, the robot's Desk login, lab network details, ultrasound images,
  recordings, rosbags, or anything showing a person. If any is found, stop, tell Daniel what and
  where, and do not commit until he decides. Such data belongs in `.gitignore`d folders, never in git.

## Workflow
- Plans live in `docs/PLAN.md`. It is the only place where status is tracked.
- A task is done only when it builds, its tests pass, and `PLAN.md` is updated.
- At the start of a session read `docs/PLAN.md`, then follow `docs/project/WORKFLOW.md`
  (Opus plans and reviews, Sonnet implements from a written task brief).
- Names of packages, topics, frames and services come from `docs/project/ARCHITECTURE.md`.
- Only Daniel settles entries in `docs/project/DECISIONS.md`; never assume an open decision.
- New code goes in separate packages under `ultrasound/` in the repo, not into `franka_*` packages.
- Code tasks are implemented by the `sonnet-implementer` subagent, given the path of the saved brief.
  Do not write the code in the main session unless Daniel asks.
- The subagent runs at medium effort. When a brief says `Effort: high`, start it with effort high.
