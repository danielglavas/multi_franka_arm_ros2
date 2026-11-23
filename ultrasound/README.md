# ultrasound/

Our own ROS 2 packages for the robotic ultrasound scanning project. The `franka_*` packages in
the repo root come from upstream and are not changed for this project; new code goes here.

All packages are Apache-2.0. At this stage they are empty skeletons that build and pass their linters.

| Package | Build type | What it is for |
|---|---|---|
| `ultrasound_probe_description` | ament_cmake | xacro, STL meshes, MuJoCo model of mount, probe and camera; calibration results |
| `ultrasound_scan_bringup` | ament_cmake | launch files, YAML configuration, RViz configuration |
| `ultrasound_scan_planning` | ament_python | surface processing, path generation, synthetic surface publisher, probe pose publisher |
| `ultrasound_scan_execution` | ament_cmake (C++) | MoveIt2 scan path executor; later the contact scan executor and safety layer |

`slicer_modules/` is not a ROS package. It holds the 3D Slicer module (Slicer's own Python 3.9)
and carries a `COLCON_IGNORE` file so that colcon does not try to build it.

`ultrasound_scan_interfaces` (custom messages and services) does not exist yet. It is created by the
first task that needs a type which no standard message fits.

See `docs/project/ARCHITECTURE.md` section 2 for the full package list.
