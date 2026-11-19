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
| D4 | Languages | proposed | Phase 1 |
| D5 | Where waypoints are clicked | proposed | Phase 3 |
| D6 | How ultrasound images reach Slicer | open | Phase 7 |
| D7 | Contact control strategy | proposed | Phase 8 |
| D8 | Own code in separate packages | proposed | Phase 0 |
| D9 | Where synthetic images are generated | proposed | Phase 4 |
| D10 | Camera model and driver | open | Phase 6 |
| D11 | Continuous or stop-and-go acquisition | open | Phase 7 |
| D12 | Slicer on the robot PC or a second PC | open | Phase 5 |
| D13 | Calibration method for image-to-probe | open | Phase 7 |

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

## D4 Languages (proposed: Python for planning nodes and Slicer; C++ for the MoveIt2 executor and controllers)

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

## D8 Own packages (proposed: all new code under `ultrasound/`)

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
