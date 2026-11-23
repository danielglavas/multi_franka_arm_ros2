# Decisions

Status values: **decided** (Daniel agreed), **proposed** (recommended, awaiting Daniel),
**open** (needs information first). When a decision changes, keep the old entry and add the new
one below it with the date, so the thesis can explain why.

Only Daniel moves a decision to "decided". Opus may add entries and recommendations.

| ID | Topic | Status | Needed by |
|---|---|---|---|
| D1 | Base repository | decided 2026-11-19 | Phase 0 |
| D2 | Workspace for the new base | decided 2026-11-19 | Phase 0 |
| D3 | Simulation: MuJoCo or fake hardware with RViz | decided 2026-11-19 | Phase 0 |
| D4 | Languages | decided 2026-11-23 | Phase 1 |
| D5 | Where waypoints are clicked | proposed | Phase 3 |
| D6 | How ultrasound images reach Slicer | open | Phase 7 |
| D7 | Contact control strategy | proposed | Phase 8 |
| D8 | Own code in separate packages | decided 2026-11-23 | Phase 0 |
| D9 | Where synthetic images are generated | proposed | Phase 4 |
| D10 | Camera model and driver | open | Phase 6 |
| D11 | Continuous or stop-and-go acquisition | open | Phase 7 |
| D12 | Slicer on the robot PC or a second PC | open | Phase 5 |
| D13 | Calibration method for image-to-probe | open | Phase 7 |
| D14 | Upgrade SlicerROS2 from `e7bf7fe` to v1.2 | open | before Phase 3 (D5) or Phase 4 |

---

## D1 Base repository (decided: switch to `tenfoldpaper/multipanda_ros2`)

**Decided 2026-11-19 by Daniel:** switch to `tenfoldpaper/multipanda_ros2`, as proposed.

Found on 2026-11-17 by reading both repositories (not built or run):

- The current fork's `humble` is an old snapshot (July 2024) of the maintained repo, which has
  111 newer commits, the latest from 2026-10-28.
- Same targets: Humble, Panda, libfranka 0.9.2, Eigen 3.3.9. Needs MuJoCo 3.2.0 (installed: 3.1.6)
  and the `mujoco_ros_pkgs` fork as a submodule (branch `wip_ros_control_humble`).
- Gains: MoveIt2 on MuJoCo (`sim_moveit.launch.py`), a gripper-less Panda, impedance controllers
  commanded over topics for sim and real, fixes to the simulated external force, sim cameras.
- Costs: different simulation structure and file layout; STL mesh change must be redone; no
  Cartesian command interface in simulation.

Alternative: stay on the current fork and write the MoveIt2-on-MuJoCo launch ourselves.
Why switch: the missing pieces are exactly the ones this project needs, and the cost of switching
grows with every commit.

## D2 Workspace (decided: new `~/ros2_ws_3`, keep `~/ros2_ws_2` until the new one drives Slicer)

**Decided 2026-11-19 by Daniel:** use a new workspace `~/ros2_ws_3`, as proposed.

Alternative: update `ros2_ws_2` in place. Rejected because the working sim-to-Slicer setup would
be gone until the new one runs. Install MuJoCo 3.2.0 into its own folder so both workspaces work.

## D3 Simulation (decided: MuJoCo as the stand-in robot, RViz as a viewer on top)

**Decided 2026-11-19 by Daniel:** MuJoCo is the simulation. RViz stays in use as a viewer, also
on its own for use cases where no simulation is needed.

RViz only displays. The real choice is fake hardware (joints jump to commands, no physics) or
MuJoCo. MuJoCo is needed because contact force is in scope, and it exposes the same controllers
as the real robot. Fallback for planning-only work if MuJoCo tracking is troublesome: fake hardware.

## D4 Languages (decided 2026-11-23: Python for planning nodes and Slicer; C++ for the MoveIt2 executor and controllers)

Daniel confirmed the proposal on 2026-11-23, while P0-T7 was being prepared. The language of
`planning_scene_publisher` (P2-T3) is still left to its brief.

- Python is quicker to learn and test, and the planning maths is not time-critical.
- MoveIt2 on Humble has no official Python API (`moveit_py` came later), so the executor uses the
  C++ `MoveGroupInterface`.
- Fallback if C++ becomes a time sink: call MoveIt2's planning service and execution action from
  Python directly, or use the third-party `pymoveit2`.

## D5 Waypoint input (proposed: RViz first, Slicer later if time allows)

- RViz: the "Publish Point" tool already publishes a clicked 3D point on a point cloud. No new code.
- Slicer: one user interface for the whole thesis, but SlicerROS2 has no point cloud type, so the
  surface would have to be exported to a file and loaded in Slicer, and waypoints sent back as a pose array.
- The generator takes waypoints on a topic either way, so the second option can be added without rework.

## D6 Ultrasound image path into Slicer (open)

Needs `LAB_CHECKLIST.md` section D. Options, to be judged once the machine is known:

| Option | Fits when | Concerns |
|---|---|---|
| PLUS toolkit and OpenIGTLink into Slicer | The machine or a frame grabber is supported by PLUS | Many vendor interfaces are Windows-only; Linux support must be checked; SlicerOpenIGTLink must be built for this Slicer |
| Frame grabber as a video device, read in ROS, bridged to Slicer | Machine has a video output | SlicerROS2 has no image type: needs a ROS-to-OpenIGTLink bridge or an added message type |
| Frame grabber read directly inside Slicer (Python) | Simplest hardware, one PC | Timestamps are Slicer's own; needs care for timing |
| Record on the machine, load files afterwards | Nothing live is possible | No live view; timing must come from stop-and-go |

Whatever is chosen, image timestamps and pose timestamps must be comparable (see D11).

## D7 Contact control (proposed: MoveIt2 for the approach, Cartesian impedance for the scan)

- Position-controlled trajectories in contact make the Panda stop with reflex errors, and give no control over force.
- Under impedance control the commanded pose lies slightly below the surface; force is roughly
  stiffness times that depth. Low stiffness along the probe axis, higher sideways.
- Stretch (P8-T6): add a force term along the probe axis regulated on the estimated external force.
- Open detail: which controller from the repo, and how the executor switches controllers. Decide in P8-T1 after reading the code.

## D8 Own packages (decided 2026-11-23: all new code under `ultrasound/` in this repo)

Daniel confirmed the proposal on 2026-11-23, while P0-T7 was being prepared.

Alternative: a separate repository for our packages. Simpler history, but two repos to keep in
step. One repo with a clear folder is enough unless the base is switched again.

## D9 Synthetic images (proposed: generated inside Slicer)

The image is cut out of a known volume at the probe pose by a Slicer module. No image transport
is needed, so Phase 4 does not depend on D6. Limitation: this cannot reveal a wrong transform by
itself (see the circularity note in `STUDENT_GUIDE.md`).

## D10 Camera (open)

Needs checklist section C: exact model, minimum working distance, whether its driver runs on the
realtime kernel. The mount design depends on the working distance.

## D11 Acquisition timing (open)

- Continuous: robot moves while images stream. Needs the delay between image and pose streams measured.
- Stop-and-go: robot stops at each pose, one image is taken. Slower, but timing no longer matters.
- Recommendation once images work: start with stop-and-go to get M-B, then try continuous.

## D12 Slicer PC (open)

Slicer, RViz and MuJoCo load the same PC that runs the 1 kHz robot control loop. If the real robot
reports communication errors while Slicer runs, move Slicer to a second PC on the same network
(ROS 2 needs the same `ROS_DOMAIN_ID` on both). Decide after P2-R0.

## D13 Image-to-probe calibration method (open)

Depends on what phantoms and tools the lab has (checklist section D). Candidates: a point target
imaged from many robot poses; a wire phantom; a method provided by PLUS. The robot is the tracker.

## D14 SlicerROS2 version (open: stay on `e7bf7fe` or upgrade to v1.2)

Added 2026-11-21 during P0-T4. Facts from `git fetch` and reading upstream `rosmed/slicer_ros2_module`
(not built or tested here):

- **Installed:** `main` at `e7bf7fe`, built against Slicer 5.6.2 on Humble, with an uncommitted
  workaround in `MRML/CMakeLists.txt` (the `rosbag2_interfaces` services `Play`, `SplitBagfile` and
  `Stop` are commented out). It loads every link mesh with `vtkSTLReader`, so the Panda needs STL
  visual meshes (done in P0-T4). It colours each link from the URDF `<material>`, or grey without one.
- **Upstream `main`:** now `4f52f10` (release v1.2, 145 commits newer). It includes:
  - DAE loading through Assimp, with one mesh per colour (v1.1, commit `c1de4b6`). The Panda
    would show its real colours with no STL files.
  - `PointCloud2` subscribers.
  - A `ROS2MotionControl` module: MoveIt planning **and execution** from Slicer, IK, trajectories, obstacles.
  - The build moves into the colcon workspace instead of the Slicer build tree.
- **Tested upstream on:** Ubuntu 24.04, Jazzy, Slicer 5.10 and 5.12. v1.0 is described as the last
  version known to work on 20.04 and tested on 22.04. Nothing says Humble or Slicer 5.6 still work.
- The rosbag2 lines our workaround comments out are still on upstream `main`, so the workaround would still be needed.
- New dependencies: `assimp` (`libassimp-dev` 5.2.2 is installed) and MoveIt libraries (installed through apt).

| Option | For | Against |
|---|---|---|
| Stay on `e7bf7fe` | Known to work here; nothing to rebuild | Grey or one-colour links; no point clouds; no MoveIt in Slicer |
| Upgrade to v1.2 against Slicer 5.6.2 | Colours, point clouds, MoveIt tools; current upstream | Unknown if it builds on Humble and 5.6.2; motion execution from Slicer must be covered by the safety rules |
| Upgrade Slicer to 5.10+ as well | Matches upstream's tested versions | Several-hour source build on 7 GB RAM; SlicerIGSIO/SlicerIGT (P0-T9) rebuilt too |

Why it matters beyond colours:
- D5 assumes "SlicerROS2 has no point cloud type". That is no longer true in v1.1+.
- MoveIt planning from Slicer could replace part of P3-T6 and help Phase 4.

Recommendation: decide after a **test build** of v1.2 against Slicer 5.6.2 in a separate copy of
the module, so the working install is not touched. If it builds and the Panda shows from the DAE
files, the STL change in `franka_description` can be undone by switching the 11 paths back.
