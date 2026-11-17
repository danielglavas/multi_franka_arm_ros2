# Architecture

This is the source of truth for names and interfaces. If code and this file disagree, one of them
is wrong: fix it in the same commit. Items marked **(proposed)** are the starting design and may
be changed by a decision in `DECISIONS.md`. Items marked **(verify)** were not tested when written.

## 1. Principles

- **Stable interfaces, replaceable sources.** Each stage talks to the next through one fixed
  interface. A synthetic source and the real source publish the same thing on the same name.
- **Own code in own packages.** Everything we write lives under `ultrasound/` in the repo. The
  `franka_*` packages are changed only when there is no other way, and each such change is listed
  in section 9. This keeps moving to a newer base repo cheap.
- **Logic separate from ROS.** Algorithms (surface processing, path generation, metrics) are plain
  Python modules with no ROS imports, tested with pytest. Nodes are thin wrappers around them.
- **Nothing moves the robot by itself.** Planning and execution are separate steps; execution
  always needs an explicit service call from a person.

## 2. Packages (proposed)

All under `ultrasound/` in the repo.

| Package | Build type | Contents |
|---|---|---|
| `ultrasound_probe_description` | ament_cmake | xacro, STL meshes, MuJoCo model of mount, probe and camera; calibration results |
| `ultrasound_scan_bringup` | ament_cmake | launch files, YAML configuration, RViz configuration |
| `ultrasound_scan_planning` | ament_python | surface processing, path generation, synthetic surface publisher, probe pose publisher |
| `ultrasound_scan_execution` | ament_cmake (C++) | MoveIt2 scan path executor; later the contact scan executor and safety layer |
| `ultrasound_scan_interfaces` | ament_cmake | custom messages or services, created only when a standard type does not fit |
| `slicer_modules/UltrasoundScan` | not a ROS package | Slicer scripted module, Python 3.9; folder carries a `COLCON_IGNORE` file |

## 3. Data flow

```
[surface source]  --/scan/surface_cloud-->  [scan_path_generator]  --/scan/path_poses-->  [scan executor]
 synthetic_surface_publisher                      ^                                         |  MoveIt2 or
 or surface_capture (real camera)                 | /clicked_point (RViz)                   v  contact controller
                                                                                          robot / MuJoCo
                                                                                            |
                                                                             robot_state_publisher --> /tf
                                                                                            |
[image source] ---------------------------------------------------------------> Slicer: UltrasoundScan module
 synthetic generator inside Slicer                                              tf lookup, image placement,
 or real ultrasound machine                                                     recording, reconstruction, segmentation
```

## 4. Nodes (proposed)

| Node | Package | Language | Phase | Role |
|---|---|---|---|---|
| `probe_pose_publisher` | planning | Python | 1 | tf lookup base to `probe_tip`, publishes `PoseStamped` |
| `planning_scene_publisher` | execution | C++ or Python | 2 | adds table and phantom collision objects from YAML |
| `scan_path_executor` | execution | C++ | 2 | plans and executes a pose list with MoveIt2 |
| `synthetic_surface_publisher` | planning | Python | 3 | stand-in for the camera |
| `scan_path_generator` | planning | Python | 3 | waypoints plus surface to probe poses |
| `surface_capture` | planning | Python | 6 | real camera cloud to base-frame surface |
| `scan_contact_executor` | execution | C++ | 8 | follows the path under impedance control, with safety checks |

## 5. Topics and services (proposed)

| Name | Type | Publisher | Notes |
|---|---|---|---|
| `/scan/surface_cloud` | `sensor_msgs/PointCloud2` | surface source | frame `panda_link0`, metres; QoS reliable, transient local (late joiners get the last cloud) |
| `/clicked_point` | `geometry_msgs/PointStamped` | RViz "Publish Point" tool | waypoint input |
| `/scan/waypoints` | `geometry_msgs/PoseArray` | generator | accepted waypoints, for display |
| `/scan/path_poses` | `geometry_msgs/PoseArray` | generator | `probe_tip` poses in `panda_link0` |
| `/scan/path` | `nav_msgs/Path` | generator | same path, for RViz |
| `/scan/probe_pose` | `geometry_msgs/PoseStamped` | `probe_pose_publisher` | for logging and learning; Slicer uses tf directly |
| `/scan/generate_path` | `std_srvs/Trigger` | generator (service) | build the path from current waypoints |
| `/scan/clear_waypoints` | `std_srvs/Trigger` | generator (service) | |
| `/scan/plan` | `std_srvs/Trigger` | executor (service) | plan only, show in RViz |
| `/scan/execute` | `std_srvs/Trigger` | executor (service) | execute the last successful plan |
| `/scan/abort` | `std_srvs/Trigger` | executor (service) | stop motion |

Parameters are declared with defaults in each node and overridden from YAML in
`ultrasound_scan_bringup/config`. No numbers hard-coded in node logic.

## 6. Frames

```
panda_link0                      robot base, the common reference for everything
  └─ ... panda_link8             flange
       └─ probe_mount            fixed, from the mount design
            ├─ probe_tip         centre of the transducer face
            └─ camera_link       camera body; the driver adds its optical frames below this
```

`probe_tip` axes **(proposed)**: z points out of the transducer face into the scanned object,
x runs along the transducer array (the lateral direction of the image), y completes the
right-handed frame (elevation, across the image plane).

Conventions:

- ROS: metres, radians, right-handed frames (REP 103). Quaternions in messages are x, y, z, w.
- Slicer: millimetres, RAS coordinates. SlicerROS2 converts for tf lookups **(verify, and record
  the exact behaviour here after task P1-T6)**.
- STL files carry no unit. Export in metres, or state `scale` in the xacro. Check that Slicer
  shows the part at the right size.

## 7. Slicer side

Transform hierarchy in the Slicer scene:

```
BaseToReference            identity unless the scene needs another reference
  └─ ProbeToBase           live, from the SlicerROS2 tf lookup (panda_link0 -> probe_tip)
       └─ ImageToProbe     constant, from spatial calibration (includes pixel-to-mm scaling)
            └─ Image       the 2D ultrasound image
PhantomToBase              pose of the ground-truth phantom volume (synthetic experiments)
```

- Slicer runs its own Python 3.9. `rclpy` cannot be imported there; ROS access goes through the
  SlicerROS2 MRML classes (`vtkMRMLROS2NodeNode` and the nodes it creates).
- SlicerROS2 has publishers and subscribers for a fixed list of message types (poses, transforms,
  wrenches, joint states, pose arrays). It has **no image and no point cloud type**. Images and
  surfaces reach Slicer another way (decisions D6 and D5).
- Robot visuals must be STL or OBJ for Slicer.
- Recording uses Sequences; reconstruction uses SlicerIGSIO Volume Reconstruction **(verify module names and API in Slicer 5.6.2)**.

## 8. Simulation and real robot

- **Two robot models exist and must agree.** ROS, MoveIt2 and Slicer use the URDF (from xacro).
  MuJoCo uses its own MJCF file. The probe, mount and camera must be added to both, and a test
  compares them (P1-T3).
- Launch files take the mode as an argument where possible, so the same nodes run on both.
- Free-space motion uses MoveIt2 with a joint trajectory controller. Contact scanning uses an
  impedance controller (D7). Switching between them is a controller switch, done by the executor.
- If the simulation publishes `/clock`, every node in that launch needs `use_sim_time:=true`
  **(verify for the MuJoCo server in the maintained repo)**. Mixed time sources break tf lookups.
- Real-robot launches are never started by the assistant without asking (see `CLAUDE.md`).

## 9. Changes made to upstream packages

Keep this list complete; it is what must be redone after moving to a new base.

| File | Change | Why |
|---|---|---|
| `franka_description/robots/.../panda_arm.xacro`, `hand.xacro` | visual meshes `.dae` to `.stl`; STL files added | Slicer only shows STL/OBJ |

## 10. Testing

| Level | Tool | Used for |
|---|---|---|
| Algorithm | pytest | surface processing, path generation, metrics |
| Description | pytest + xacro | frames exist, offsets, URDF and MJCF agree |
| Node | pytest with rclpy, or `launch_testing` | topics appear, services respond |
| C++ helpers | gtest | executor helper functions |
| Slicer module | the module's own test class, run inside Slicer | hierarchy, reslicing, metrics |
| System | written manual procedure with a measured number | anything involving MuJoCo, MoveIt2 or the real robot |

"Tests pass" means: the new package's tests pass and no test fails that passed in `TEST_BASELINE.md`.

## 11. Naming

- Packages `ultrasound_*`, nodes and topics in `snake_case`, topics under `/scan/`.
- Names say what the thing is for: `probe_tip_pose_in_base`, not `T` or `pose2`.
- Transform variables say both frames: `probe_tip_from_flange` or `FlangeToProbeTip`, one style per language.
- Every source file starts with a short comment saying what it is for; every function has a
  docstring or comment; non-obvious maths gets a comment with the formula.
