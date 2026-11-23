# slicer_modules/

This folder is not a ROS package. The `COLCON_IGNORE` file next to this README tells colcon to skip it.

The 3D Slicer scripted module `UltrasoundScan` will be added here in task P4-T2. It runs inside
3D Slicer on Slicer's own Python 3.9, so it cannot import `rclpy`; it talks to ROS 2 only through
the SlicerROS2 module.
