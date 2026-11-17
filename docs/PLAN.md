# PLAN: robotic ultrasound scanning with Panda, MoveIt2 and 3D Slicer

Last updated: 2026-11-17 (initial version)
Thesis deadline: **2027-05-01**. Real robot is required.

This file is the **only** place where progress is tracked. The other documents explain; they do
not carry status.

| Document | Purpose | Reader |
|---|---|---|
| `docs/project/ROADMAP.md` | One-page overview | everyone |
| `docs/PLAN.md` (this file) | Phases, tasks, acceptance checks, status | Opus, Daniel |
| `docs/project/ARCHITECTURE.md` | Packages, nodes, topics, frames, conventions | Opus, Sonnet |
| `docs/project/DECISIONS.md` | Decisions made, proposed and open | Opus, Daniel |
| `docs/project/WORKFLOW.md` | How Opus plans and Sonnet codes; task brief template | Opus, Sonnet |
| `docs/project/STUDENT_GUIDE.md` | Learning path, tips, pitfalls | Daniel |
| `docs/project/LAB_CHECKLIST.md` | What to find out and do in the lab | Daniel |

## How to use this file

- Status marks: `[ ]` not started, `[~]` in progress, `[x]` done, `[-]` dropped (say why).
- A task is done only when it builds, its tests pass, its "Done when" check was actually
  performed, and this file is updated.
- Task types: **code** (Sonnet implements from a brief), **config** (small file changes),
  **lab** (Daniel, at the robot), **learn** (Daniel), **decision** (Daniel decides, Opus prepares).
- Tasks in Phases 0 to 4 are specified. Tasks in Phases 5 to 9 are outlines: before starting one,
  Opus writes a task brief (template in `WORKFLOW.md`) using the lab information that exists by then.
- Dates are estimates made on 2026-11-17. Adjust them here when reality differs; do not silently slip.
- When a task changes an interface (topic, frame, parameter), update `ARCHITECTURE.md` in the same commit.
- Facts marked **(verify)** were not tested when this plan was written.

## Timeline

| Phase | Content | Window | Needs lab hardware |
|---|---|---|---|
| 0 | Foundation: repo, workspace, baseline | Nov 17 to Nov 29 | no |
| 1 | Probe model and first ROS 2 nodes | Nov 30 to Dec 13 | no |
| 2 | MoveIt2 on the MuJoCo simulation | Dec 14 to Jan 3 | no (R0: yes) |
| 3 | Surface and scan path, synthetic camera | Jan 4 to Jan 17 | no |
| 4 | Slicer pipeline with synthetic images | Jan 18 to Feb 6 | no |
| 5 | Real probe mount on the real robot | parallel, complete by Jan 30 | yes |
| 6 | Real 3D camera | Feb 1 to Feb 13 | yes |
| 7 | Real ultrasound images | Feb 8 to Feb 27 | yes |
| 8 | Contact-controlled scanning | Feb 15 to Mar 19 | sim first, then yes |
| 9 | Integration, evaluation, segmentation | Mar 15 to Apr 3 | yes |
| 10 | Writing and buffer | Apr 4 to May 1 | no |

Checkpoints:

- **R0, by Dec 19:** real robot moves in free space under our launch files and Slicer mirrors it.
- **M-A, by Feb 6:** full synthetic pipeline: click a path, robot scans in simulation, Slicer
  reconstructs a volume from synthetic images and segments it.
- **M-B, by Feb 27:** first reconstructed volume from real ultrasound images with robot poses.
- **M-C, by Mar 26:** full real pipeline on the abdomen phantom.
- **Feature freeze, Apr 3:** after this date only bug fixes, experiments and writing.

If behind schedule, cut in this order (see "Cut lines" at the end).

---

## Phase 0: Foundation (Nov 17 to Nov 29)

Goal: a known-good, maintained base that builds, runs sim to Slicer, and has a recorded test baseline.

- [ ] **P0-T1 Confirm base decisions** (decision, Daniel). Confirm or change D1 (switch to
  `tenfoldpaper/multipanda_ros2`), D2 (new workspace `~/ros2_ws_3`, old one kept as fallback),
  D3 (MuJoCo is the main simulation) in `DECISIONS.md`.
  Done when: the three decisions are marked "decided".
- [ ] **P0-T2 Build the maintained repo in a new workspace** (config, Daniel with Opus guiding).
  Steps in outline: install MuJoCo 3.2.0 into its own prefix (for example
  `~/Libraries/mujoco-3.2.0`, not `/usr/local`, so the old workspace keeps working with 3.1.6);
  clone the fork into `~/ros2_ws_3/src`; add the maintained repo as a remote; bring `ultrasound`
  onto its `humble` (the fork's `humble` is an ancestor, so this is a rebase of one commit plus
  these docs); `git submodule update --init --recursive` for `mujoco_ros_pkgs`; `rosdep install`;
  build Release.
  Done when: `ros2 launch franka_bringup franka_sim.launch.py` opens MuJoCo with one Panda in the
  new workspace, and the old workspace still launches too.
- [ ] **P0-T3 Install missing ROS packages** (config, Daniel, needs sudo).
  At least `ros-humble-moveit-simple-controller-manager` and `ros-humble-joint-trajectory-controller`
  (or `ros-humble-ros2-controllers`). Let `rosdep install --from-paths src --ignore-src -r` list the rest.
  Done when: `ros2 launch franka_moveit_config sim_moveit.launch.py` starts `move_group` without
  plugin-not-found errors and a motion planned in RViz executes in MuJoCo.
- [ ] **P0-T4 Restore the Slicer connection on the new base** (config). Redo the DAE to STL change
  for the moved xacro files (`franka_description/robots/common/panda_arm.xacro`, `hand.xacro`).
  Done when: Slicer shows the Panda from the new workspace and it follows the simulation.
- [ ] **P0-T5 Move the Claude Code setup** (config). `CLAUDE.md`, `.claude/settings.json` and
  `~/start_env.sh` point to the new workspace, remotes and git rules.
  Done when: a new Claude Code session in the new workspace loads `CLAUDE.md` without warnings.
- [ ] **P0-T6 Record the test baseline** (config). Run `colcon test` on the untouched packages and
  save the summary to `docs/project/TEST_BASELINE.md` (which packages fail which linters/tests).
  Done when: the file exists and "tests pass" is defined against it (no new failures).
- [ ] **P0-T7 Create the package skeleton** (code). Folder `ultrasound/` in the repo with empty but
  building packages as listed in `ARCHITECTURE.md`, each with license, description, and a passing
  lint/test setup.
  Done when: `colcon build` and `colcon test` succeed for the new packages.
- [ ] **P0-T8 Explain the 2026-11-13 tf failure** (config). The Slicer log showed "panda_link0 does
  not exist" for all tf lookups. Reproduce or rule out: check `ros2 run tf2_ros tf2_echo
  panda_link0 panda_link8`, `use_sim_time`, start order, tf prefix.
  Done when: the cause is written in `STUDENT_GUIDE.md` under "Known problems", or it is shown not to recur.
- [ ] **P0-T9 Load SlicerIGSIO and SlicerIGT into Slicer** (config, Daniel). They are built in
  `~/slicer/modules` but not on Slicer's module path **(verify they were built against this Slicer 5.6.2)**.
  Done when: the "Volume Reconstruction" and "Sequences" modules are available in Slicer.
- [ ] **P0-T10 Collect lab information** (lab, Daniel). Work through `LAB_CHECKLIST.md`.
  Done when: sections A to D of the checklist are filled in.

## Phase 1: Probe model and first ROS 2 nodes (Nov 30 to Dec 13)

Goal: the simulated robot carries a placeholder probe that Slicer shows, and Daniel has written
and understood a first node.

- [ ] **P1-T1 ROS 2 basics** (learn, Daniel). The list in `STUDENT_GUIDE.md`, section "Learning path", block 1.
  Done when: Daniel can explain node, topic, service, parameter, launch file, tf frame, and has run the tutorials.
- [ ] **P1-T2 Placeholder probe description** (code). Package `ultrasound_probe_description`:
  xacro for mount, probe and camera as simple shapes, with all dimensions as named xacro
  properties in one file so real values drop in later. Frames `probe_mount`, `probe_tip`,
  `camera_link` as defined in `ARCHITECTURE.md`. Visual meshes as STL in metres.
  Done when: pytest confirms the xacro parses with and without probe, the three frames exist, and
  `probe_tip` is at the configured offset from `panda_link8`.
- [ ] **P1-T3 Probe in the MuJoCo model** (code). MuJoCo uses its own model file (MJCF), separate
  from the URDF. Add the probe body with matching geometry, mass and inertia to a copy of the
  gripper-less scene.
  Done when: a test compares probe mass and tip offset between URDF and MJCF and they agree; the
  simulation runs with the probe.
- [ ] **P1-T4 Bringup launch with probe** (code). `ultrasound_scan_bringup/launch/probe_sim.launch.py`.
  Done when: one command starts the simulation with the probe, and Slicer shows the probe on the flange.
- [ ] **P1-T5 Probe pose publisher node** (code, learning task). Python node that looks up
  `panda_link0` to `probe_tip` in tf and publishes it as `geometry_msgs/PoseStamped`. Brief is
  written out in `WORKFLOW.md` as example 1.
  Done when: unit tests pass and `ros2 topic echo /scan/probe_pose` shows the pose changing with the robot.
- [ ] **P1-T6 Probe transform in Slicer, verified** (config, Daniel). Create the tf lookup in
  Slicer and compare its numbers with `tf2_echo` at three different robot poses.
  Done when: position agrees within 0.1 mm and rotation within 0.1 degree after unit conversion,
  and the unit and axis convention found is written into `ARCHITECTURE.md`.

## Phase 2: MoveIt2 on the simulation (Dec 14 to Jan 3)

Goal: a list of poses can be planned and executed on the simulated robot with the probe.

- [ ] **P2-T1 MoveIt2 concepts** (learn, Daniel). `STUDENT_GUIDE.md`, learning block 2.
- [ ] **P2-T2 MoveIt2 configuration with probe** (code). SRDF with planning group ending at
  `probe_tip`, collision pairs disabled between mount and neighbouring links, and
  `probe_sim_moveit.launch.py` derived from the repo's `sim_moveit.launch.py`.
  Done when: the SRDF test passes and a pose goal for `probe_tip` set in RViz is planned and executed in MuJoCo.
- [ ] **P2-T3 Planning scene from a file** (code). Node that adds the table and a placeholder
  phantom as collision objects from a YAML file.
  Done when: objects appear in RViz and a plan through the table is refused.
- [ ] **P2-T4 Scan path executor** (code). C++ node using `MoveGroupInterface`: receives a
  `PoseArray`, plans the approach, computes a Cartesian path through the poses, and executes
  **only** when its `execute` service is called. Refuses when the achieved path fraction is below a threshold.
  Done when: a hand-written list of five poses is executed in simulation; a path with an
  unreachable pose is refused with a clear message.
- [ ] **P2-T5 Tracking error measurement** (code). Script that records commanded and actual
  `probe_tip` positions during a run and reports RMS and maximum error.
  Done when: the number is produced for the P2-T4 run and written into the thesis notes.
- [ ] **P2-R0 Real robot smoke test** (lab, Daniel with supervisor). Free-space only, gripper or
  bare flange is fine. Bring up the real robot with the repo's real launch, mirror it in Slicer,
  execute one small MoveIt2 motion at low speed.
  Done when: checkpoint R0 is met and anything surprising is noted in `STUDENT_GUIDE.md`.

## Phase 3: Surface and scan path with a synthetic camera (Jan 4 to Jan 17)

Goal: clicking points on a surface produces a probe path that follows the surface, and the
simulated robot executes it. The real camera later replaces only the point cloud source.

- [ ] **P3-T1 Synthetic surface publisher** (code). Publishes a `PointCloud2` on
  `/scan/surface_cloud` in frame `panda_link0`, sampled from analytic shapes (plane, cylinder
  roughly like a forearm, ellipsoid cap roughly like an abdomen) with configurable noise.
  Done when: the cloud shows in RViz at the same place as the phantom object in MuJoCo.
- [ ] **P3-T2 Surface processing library** (code). Pure Python, no ROS imports: crop, downsample,
  normal estimation with consistent orientation.
  Done when: tests on the analytic shapes give a mean normal error below 5 degrees with 1 mm noise.
- [ ] **P3-T3 Path generation library** (code). Pure Python: project clicked points onto the
  surface, resample at a fixed spacing along the surface, build poses with probe axis along the
  inward normal and image axis along the travel direction, add approach and retreat poses.
  Brief written out in `WORKFLOW.md` as example 2.
  Done when: tests on plane and cylinder pass (position on surface within 1 mm, axis within 5 degrees, no orientation flips).
- [ ] **P3-T4 Scan path generator node** (code). ROS wrapper: subscribes to the cloud and to
  clicked points, offers services to generate and clear, publishes `PoseArray`, `Path` and markers.
  Done when: clicking three points in RViz shows a path lying on the surface.
- [ ] **P3-T5 End to end in simulation, hovering** (config). Click, generate, plan, execute, with
  the path offset 10 mm above the surface so no contact is needed yet.
  Done when: the simulated probe follows the surface at the offset; tracking error is recorded.
- [ ] **P3-T6 Waypoints from Slicer** (decision D5, then code). Only if D5 is decided for Slicer.

## Phase 4: Slicer pipeline with synthetic images (Jan 18 to Feb 6)

Goal: checkpoint M-A. Everything after the robot pose works on artificial data: image placement,
recording, volume reconstruction, comparison with ground truth, basic segmentation.

- [ ] **P4-T1 Slicer Python basics** (learn, Daniel). `STUDENT_GUIDE.md`, learning block 3.
- [ ] **P4-T2 Slicer module skeleton** (code). Scripted module `UltrasoundScan` in the repo folder
  `ultrasound/slicer_modules/` (Python 3.9, not a ROS package). Connects to the SlicerROS2 node,
  creates the tf lookup, builds the transform hierarchy from `ARCHITECTURE.md`.
  Done when: the module loads, a button creates the hierarchy, and its self-test passes.
- [ ] **P4-T3 Ground-truth phantom volume** (code). Generated volume with inclusions of known
  size and position (spheres, cylinder), placed in the robot base frame at the same pose as the
  simulated phantom.
  Done when: the volume appears in Slicer where the robot scans, and its parameters are stored in one file.
- [ ] **P4-T4 Synthetic ultrasound image generator** (code). On each update, cut the image plane
  defined by the probe pose and `ImageToProbe` out of the phantom volume and show it as a 2D image
  that moves with the probe.
  Done when: moving the robot changes the image, and the image matches the phantom at three checked poses.
- [ ] **P4-T5 Recording** (code). Record image and transform together during a scan (Sequences).
  Done when: a recorded scan replays in Slicer with image and pose in step.
- [ ] **P4-T6 Volume reconstruction** (code). Scripted reconstruction from the recording (SlicerIGSIO).
  Done when: a volume is produced from a simulated sweep.
- [ ] **P4-T7 Comparison with ground truth** (code). Metrics between reconstruction and phantom in
  the scanned region. Include a perturbation experiment: add a known error to `ImageToProbe` and
  show the metrics degrade. (Needed because slicing and reconstructing with the same transform
  always agree; see `STUDENT_GUIDE.md`, "The circularity trap".)
  Done when: a table of metrics for zero and non-zero perturbation exists.
- [ ] **P4-T8 Basic segmentation** (code). Scripted threshold-based segmentation of the
  reconstructed volume; measured inclusion sizes compared with the known sizes.
  Done when: size errors are reported for each inclusion.

## Phase 5: Real probe mount on the real robot (parallel, complete by Jan 30)

Outline. Opus writes briefs when `LAB_CHECKLIST.md` sections A, B and E are filled.

- [ ] **P5-T1 Mount design** (lab, Daniel). Requirements in `STUDENT_GUIDE.md`, "Designing the mount".
- [ ] **P5-T2 Real dimensions into the models** (code). xacro, MJCF, SRDF, STL meshes. The Phase 1 tests must still pass.
- [ ] **P5-T3 Payload configuration** (lab). Mass, centre of mass and inertia of mount, probe and
  camera set on the robot. Done when: the external force estimate is near zero at five different free-space poses.
- [ ] **P5-T4 Real bring-up with probe** (lab). Free-space MoveIt2 motion, Slicer mirrors.
- [ ] **P5-T5 Tool-tip check** (lab). Rotate about `probe_tip` while it touches a fixed point.
  Done when: tip wander is measured and recorded.

## Phase 6: Real 3D camera (Feb 1 to Feb 13)

Outline. Needs checklist section C and decision D10.

- [ ] **P6-T1 Camera driver** (lab/config). Running on this realtime-kernel machine, publishing a point cloud.
- [ ] **P6-T2 Camera-to-flange calibration** (lab). Result stored in the description package.
- [ ] **P6-T3 Surface capture node** (code). Captures one or more clouds, transforms to the base
  frame, crops to the workspace, filters, publishes on `/scan/surface_cloud` (same interface as P3-T1) and saves to a file.
- [ ] **P6-T4 Accuracy check** (lab). Touch surface points with the probe tip and compare with the cloud. Done when: the error is recorded.

## Phase 7: Real ultrasound images (Feb 8 to Feb 27)

Outline. Needs checklist section D and decision D6.

- [ ] **P7-T1 Decide the image path** (decision D6).
- [ ] **P7-T2 Live images in Slicer** (code/config), no pose yet.
- [ ] **P7-T3 Fix and document imaging settings** (lab): depth, gain, pixel spacing.
- [ ] **P7-T4 Spatial calibration** (lab + code): `ImageToProbe`, with residual error reported.
- [ ] **P7-T5 Timing** (decision D11): temporal calibration, or stop-and-go acquisition.
- [ ] **P7-T6 First real reconstruction** (lab): robot guided by hand over the phantom, images
  recorded with poses, volume reconstructed. This is checkpoint M-B and needs no path planning.

## Phase 8: Contact-controlled scanning (Feb 15 to Mar 19)

Outline. The simulation part (T1 to T4) needs no lab and may start earlier if Phase 4 finishes early.

- [ ] **P8-T1 Decide the control strategy** (decision D7), after reading the repo's impedance controllers.
- [ ] **P8-T2 Contact scan executor in simulation** (code): switch controllers, stream the target pose along the path, log the force estimate.
- [ ] **P8-T3 Tuning in simulation** (code/config): stiffness and offset against resulting force; force plot.
- [ ] **P8-T4 Safety layer** (code): force limit, speed limit, workspace bounds, watchdog, each proven by a deliberate fault in simulation.
- [ ] **P8-T5 Real robot on a rigid dummy, then the phantom** (lab): low stiffness first; force checked against a scale.
- [ ] **P8-T6 Force regulation** (stretch).

## Phase 9: Integration, evaluation, segmentation (Mar 15 to Apr 3)

- [ ] **P9-T1 Full real pipeline on the abdomen phantom** (lab). Checkpoint M-C.
- [ ] **P9-T2 Experiment protocol and recording** (config): every run stores rosbag, Slicer scene, settings and git commit.
- [ ] **P9-T3 Evaluation metrics** (code): see `ROADMAP.md`, "What the thesis measures".
- [ ] **P9-T4 Segmentation of real volumes** (code).
- [ ] **P9-T5 Arm scan demonstration** (stretch, lab): only with supervisor approval and the P8-T4 limits proven.

## Phase 10: Writing and buffer (Apr 4 to May 1)

- [ ] **P10-T1 Thesis text complete** (Daniel). Writing starts in January; this phase finishes it.
- [ ] **P10-T2 Repeat any experiment whose data is weak.**
- [ ] **P10-T3 Clean repository, final README, tagged release.**

---

## Cut lines (if behind schedule, drop in this order)

1. P9-T5 arm demonstration.
2. P8-T6 force regulation: keep plain impedance contact.
3. P3-T6 waypoints from Slicer: keep RViz clicking.
4. Multi-view camera capture: one view only.
5. P7-T5 temporal calibration: use stop-and-go acquisition.
6. Automated segmentation: do it by hand in Slicer's Segment Editor and report it as manual.

Never cut: R0, M-B, the safety layer P8-T4, the evaluation P9-T3.

## Change log

- 2026-11-17: initial plan written (Fable 5.2) from the setup investigation and discussion with Daniel.
