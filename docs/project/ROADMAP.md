# Roadmap overview

Status lives in `docs/PLAN.md`. This page only explains the shape of the project.

## What is being built

A Franka Emika Panda carries an ultrasound probe and a 3D camera. The system:

1. captures the surface of the object on the table with the 3D camera;
2. lets the user mark a scan path on that surface;
3. moves the probe along the path with MoveIt2, keeping controlled contact;
4. sends the probe pose to 3D Slicer through SlicerROS2;
5. places each ultrasound image in 3D using that pose;
6. reconstructs a 3D volume from the images;
7. segments the volume.

Target object: an abdomen phantom. Stretch: a forearm, with approval and proven safety limits.

## The pipeline

```
3D camera --> surface point cloud --> scan path --> MoveIt2 / contact controller --> Panda
                                         ^                                            |
                                   user waypoints                                 robot pose (tf)
                                                                                      |
ultrasound machine --> images ------------------------------------------------> 3D Slicer
                                                                 place images, reconstruct, segment
```

## Strategy

- **Thin end-to-end chain first.** Phases 1 to 4 build the whole chain in simulation with stand-in
  data (placeholder probe, synthetic surface, synthetic images). Real hardware then replaces one
  stand-in at a time, behind an interface that does not change.
- **Real robot early.** A free-space smoke test (R0) happens in December, long before contact.
- **Every phase ends in a measurement.** The numbers collected along the way become the thesis results.

## Phases

| Phase | Result | By |
|---|---|---|
| 0 Foundation | Maintained repo builds, sim to Slicer works, test baseline recorded | Nov 29 |
| 1 Probe model | Simulated robot carries a placeholder probe, visible in Slicer; first own node | Dec 13 |
| 2 MoveIt2 | A list of probe poses is planned and executed in simulation; real robot smoke test | Jan 3 |
| 3 Scan path | Clicked points become a surface-following probe path | Jan 17 |
| 4 Slicer pipeline | Synthetic images, reconstruction, comparison, segmentation (M-A) | Feb 6 |
| 5 Real mount | Real probe and camera mounted, modelled, payload configured | Jan 30 |
| 6 Real camera | Calibrated camera supplies the surface | Feb 13 |
| 7 Real images | Calibrated ultrasound images placed by robot pose; first real volume (M-B) | Feb 27 |
| 8 Contact | Probe follows the path with controlled contact, with a safety layer | Mar 19 |
| 9 Evaluation | Full pipeline on the phantom (M-C), metrics, segmentation | Apr 3 |
| 10 Writing | Thesis finished | May 1 |

## What the thesis measures

- **Pose chain accuracy:** agreement between robot pose in ROS and in Slicer; tool-tip accuracy.
- **Path tracking:** commanded against actual probe path, in simulation and on the robot.
- **Surface accuracy:** camera surface against points touched with the probe tip.
- **Calibration quality:** residual error of the image-to-probe calibration.
- **Contact force:** set force against measured force along the scan.
- **Reconstruction accuracy:** sizes and positions of known phantom structures in the reconstructed volume.
- **Segmentation quality:** overlap with ground truth where it is known.

## Biggest risks

| Risk | Why it matters | Mitigation |
|---|---|---|
| Ultrasound image access unknown | Decides the whole image path into Slicer | Lab checklist section D first; synthetic images keep work moving |
| Three calibrations stack up | They dominate reconstruction quality | Measure each one separately; perturbation experiment in Phase 4 |
| Contact control on the real robot | Position control in contact triggers robot faults | Impedance control, low stiffness, rigid dummy before phantom |
| Realtime kernel and camera driver | Camera kernel modules may not build on this kernel | Check early (checklist C); user-space driver backend as fallback |
| SlicerROS2 version mismatch | Jazzy-era module on Humble | Keep to features already proven; note any failure in the guide |
| Time | Hardware phases sit in February and March | R0 in December; cut lines in `PLAN.md` |
