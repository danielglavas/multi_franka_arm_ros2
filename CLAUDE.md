# Panda + ultrasound probe + 3D Slicer

## Working with Daniel (student; Claude is coder and mentor)
- Before doing something, explain what, why and how.
- When making a choice, briefly name the alternatives and why this one won.
- All code gets comments explaining what it does. Names (variables, functions, topics) describe their purpose.
- Investigate and propose first; change files only after approval.
- Before presenting a proposal or plan, re-read the files it names and check each claim (paths,
  names, line contents) against them. Say which claims were not checked.
- Change files with Edit/Write, not shell redirection (`>>`, `sed -i`), so the edit-check hook sees them.

## System
Goal chain: Franka Emika Panda -> MoveIt2 -> ROS 2 Humble -> SlicerROS2 module -> 3D Slicer.
The end effector will be an ultrasound probe (not modelled yet; `hand.xacro` gripper is still the only EOAT).
- `franka_bringup franka_sim.launch.py` runs the Panda in MuJoCo (not fake hardware, not the real robot).
- SlicerROS2 makes Slicer a ROS 2 node: it reads the URDF from the `robot_description` parameter of
  `robot_state_publisher` and moves the links via tf2. The installed SlicerROS2 (`e7bf7fe`) loads
  meshes with `vtkSTLReader` only; v1.1+ also loads OBJ and DAE (via Assimp). See D14 in `DECISIONS.md`.
- Slicer uses its own Python 3.9, so `rclpy` cannot be imported inside Slicer; use the module's MRML classes.
- MoveIt2 (apt, 2.5.10) is installed with `moveit_simple_controller_manager` and
  `joint_trajectory_controller`. `franka_moveit_config/launch/sim_moveit.launch.py` exists but is untested (P0-T3).
- MuJoCo 3.2.0 is in `~/Libraries/mujoco-3.2.0`. MuJoCo 3.1.6 in `/usr/local` belongs to the old workspace only.

## Where things live
- This repo (`~/ros2_ws_3/src/multi_franka_arm_ros2`): main repo, based on `tenfoldpaper/multipanda_ros2`.
  All project work happens here. `mujoco_ros_pkgs/` is a git submodule (tenfoldpaper's fork).
- `~/ros2_ws_2/src/multi_franka_arm_ros2`: old base, read-only fallback with outdated git history.
  Never pull, push, edit or start sessions there.
- `~/ros2_ws/src/slicer_ros2_module`: SlicerROS2, separate workspace, kept separate on purpose.
  It has an uncommitted Humble workaround in `MRML/CMakeLists.txt`; never discard it.
- `~/slicer/Slicer`: Slicer 5.6.2 source. `~/slicer/Slicer-SuperBuild-Release/Slicer-build`: build tree
  with the `Slicer` launcher. Very large: search only targeted subfolders.
- The two workspaces do not depend on each other; each chains only `/opt/ros/humble`.

## Sourcing and running
- Claude's shell has no ROS environment and does not read `~/.bashrc` (so no MuJoCo 3.2.0 or
  Eigen 3.3.9 paths). Prefix every `ros2`/`colcon` command with `source ~/start_env.sh &&`.
  That file sets the library paths, sources ROS and the workspace, and defines `start_panda_sim` and `start_slicer`.
- Terminal 1: `source ~/start_env.sh`, then `start_panda_sim` (= `ros2 launch franka_bringup franka_sim.launch.py`).
- Terminal 2 (after the sim is up): source as above, `cd ~/slicer/Slicer-SuperBuild-Release/Slicer-build && ./Slicer`,
  then add the robot in the ROS2 module.
- `~/ros2_ws/install/setup.bash` is only needed to build or test the Slicer module, not to run Slicer with the Panda.

## Build and test
- Build: `cd ~/ros2_ws_3 && MAKEFLAGS="-j4" colcon build --parallel-workers 2 --cmake-args -DCMAKE_BUILD_TYPE=Release`
  (add `--packages-select <pkg>`). The limits are needed: the PC has 7 GB RAM.
- Builds print many warnings from upstream code; colcon's "stderr output" is not a failure. Only "Failed"/"Aborted" is.
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
- `origin` = Daniel's fork (`danielglavas/multi_franka_arm_ros2`), `upstream` = `tenfoldpaper/multipanda_ros2`.
  Never push to `upstream`. Do not use GitHub's "Sync fork" button: the fork's GitHub parent is
  still `yilmazabdurrah/multi_franka_arm_ros2`, not tenfoldpaper.
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
