# Student guide

Written for Daniel. It collects what is worth knowing before you meet it the hard way: what to
learn and in which order, how the pieces really work, and where projects like this usually lose
time. Read sections 1 to 3 now; read the others when their phase starts.

Statements marked **(verify)** are from general knowledge of these tools and were not tested on
your machine. Treat them as strong hints, and write the confirmed fact here when you find out.

Contents:
1. How to work with the AI models
2. Learning path
3. Daily routine and debugging
4. ROS 2 pitfalls
5. Robot model and simulation
6. MoveIt2
7. Scan path and the camera
8. 3D Slicer and SlicerROS2
9. Ultrasound images and calibration
10. Contact and force
11. The real robot and safety
12. Designing the mount
13. Thesis habits
14. Known problems on this machine

---

## 1. How to work with the AI models

- **You own the decisions.** The models propose; `DECISIONS.md` is yours. If a model assumes a
  decision, stop it and decide.
- **Make it prove things.** "It builds and the test passes" is a claim; ask for the command output.
  For anything involving the simulator, Slicer or the robot, you run it and look.
- **Ask "what would break this?"** after every piece of code. The answer is usually the next test.
- **Keep sessions short and focused.** One task per session works better than a long wandering
  one. The plan file carries the memory, so a fresh session loses nothing.
- **Do not let it code what you cannot explain.** The thesis defence is yours. After each task,
  explain the code back in your own words; if you cannot, ask until you can. This is also the
  best check that the code is right.
- **Type some of it yourself.** For the first node (P1-T5) and one algorithm (the frame
  construction in P3-T3), write the code by hand with the model reviewing. Reading alone does not stick.
- **Give it facts, not impressions.** Paste the exact error text, the exact command, and what
  you expected. "It does not work" costs a round trip.
- **When a model is confidently wrong about a tool's API, look at the source.** You have the
  Slicer source, the SlicerROS2 source and the Franka packages on disk. Source beats memory.

## 2. Learning path

Do each block just before its phase. Sources named here are the official ones; search for the
title rather than trusting a link.

**Block 1: ROS 2 basics (before Phase 1, about 3 to 4 days)**
From the ROS 2 Humble documentation, "Tutorials":
- Beginner CLI tools: nodes, topics, services, parameters, actions, `rqt_graph`, launching, `ros2 bag`.
- Beginner client libraries: creating a workspace and a package, writing a publisher and
  subscriber in Python, a service and client in Python, using parameters in a class.
- Intermediate: launch files; the whole **tf2** section (this project lives on tf); the URDF
  tutorials including xacro.
- Then the `ros2_control` documentation's "Getting started" page, to understand hardware
  interface, controller manager and controllers. You will not write a hardware interface, but you
  will switch controllers.

Check yourself: can you draw, from memory, which node publishes `/tf` for the robot links and
where it gets the joint angles from?

**Block 2: MoveIt2 (before Phase 2, about 2 days)**
From the MoveIt 2 Humble tutorials: "Getting started", "MoveIt quickstart in RViz", "Move Group
C++ interface", "Planning scene". Concepts to hold: planning group, end-effector link, SRDF,
planning scene and collision objects, Cartesian path and its "fraction", controller managers.

**Block 3: Slicer scripting (before Phase 4, about 3 days)**
- Slicer documentation, Developer guide: "Python FAQ", "Script repository" (your most used page),
  and how to create a scripted module with the Extension Wizard.
- The SlicerROS2 documentation, especially the pages on tf2, robots, and publishers and
  subscribers. The same text is on disk in `~/ros2_ws/src/slicer_ros2_module/docs/pages`.
- SlicerIGT tutorials on tracked ultrasound and volume reconstruction. They assume an optical
  tracker; for you the robot is the tracker, everything else carries over.

**Block 4: read when needed**
- Franka control interface documentation: robot state fields, external force estimates, error
  recovery, the meaning of reflex errors.
- MuJoCo documentation: the modelling chapter (bodies, geoms, inertial, contact).
- A survey paper on freehand 3D ultrasound calibration, before Phase 7. Ask your supervisor for the lab's standard reference.

## 3. Daily routine and debugging

**Start of a work session**
1. `source ~/start_env.sh` in each terminal (update the script after the repo switch).
2. Start the simulation, wait until it is up, then start Slicer. Slicer reads the robot model
   from the running `robot_state_publisher`, so the order matters.
3. `git status` before touching anything.

**The five commands that answer most questions**
- `ros2 node list` and `ros2 topic list`: is the thing running at all?
- `ros2 topic hz <topic>` and `ros2 topic echo <topic> --once`: is data flowing, and is it sane?
- `ros2 run tf2_ros tf2_echo <parent> <child>`: is the transform there, and what is it?
- `ros2 run tf2_tools view_frames`: writes a PDF of the whole frame tree.
- `ros2 control list_controllers`: which controllers are active?

**Debugging order.** When something does not work, check in this order and stop at the first
failure: is it built (did you rebuild after editing)? is it sourced (in this terminal)? is the
node running? is the topic there? is data on it? are the frames right? are the units right?
Most lost hours are in the first two.

**Rebuild reminders**
- The workspace is not a symlink install: editing a launch file, xacro, YAML or mesh needs a
  rebuild before it takes effect. Consider `colcon build --symlink-install` for the new
  workspace; then only C++ and new files need rebuilding **(verify it suits the Franka packages)**.
- After building, open a new terminal or source again.
- A build that fails in a strange way after big changes: delete that package's folders in
  `build/` and `install/` and rebuild it. Do this yourself; the assistant is not allowed `rm -rf`.

## 4. ROS 2 pitfalls

- **Quality of service mismatch.** A subscriber that asks for "reliable" gets nothing from a
  "best effort" publisher, silently. Sensor drivers (cameras) usually publish best effort. If a
  topic has a publisher and a subscriber but no data arrives, run `ros2 topic info -v <topic>` and compare.
- **Latched data.** A point cloud published once is missed by a node that starts later, unless
  both sides use "transient local" durability. `/scan/surface_cloud` is specified that way.
- **Simulated time.** If the simulator publishes `/clock`, nodes need `use_sim_time:=true`. A node
  on wall-clock time asking tf for a simulated-time transform gets "extrapolation" errors or
  nothing. This is a prime suspect for transform lookups that mysteriously fail.
- **Domain ID.** ROS 2 nodes on the same network find each other. In a lab with other ROS users,
  your nodes and theirs can cross-talk, which with a robot is dangerous. Set a `ROS_DOMAIN_ID`
  of your own, or `ROS_LOCALHOST_ONLY=1` while everything runs on one PC. It must be the same in
  every terminal, including the one that starts Slicer.
- **Callbacks block each other.** In a Python node with the default executor, one slow callback
  delays all others. Keep callbacks short; do heavy work outside them.
- **Quaternion order.** ROS messages are x, y, z, w. Some libraries use w, x, y, z. Check every time.
- **Transform direction.** "The transform from A to B" is said both ways by different people and
  libraries. Never trust the wording; test with a point whose answer you know. In tf,
  `lookup_transform(target, source)` gives the pose of `source` expressed in `target`.

## 5. Robot model and simulation

- **There are two robot models.** The URDF (generated from xacro) is used by ROS, MoveIt2 and
  Slicer. MuJoCo uses a separate MJCF file. They are maintained by hand. If you add the probe to
  one and forget the other, the simulation and the planner disagree about where the tip is, and
  everything downstream is subtly wrong. Task P1-T3 adds a test for this; keep it alive.
- **Mass and inertia matter in MuJoCo and on the real robot.** A placeholder probe with zero or
  absurd inertia makes the simulation unstable. Use plausible values even for the placeholder.
- **STL has no units.** CAD programs export millimetres; ROS expects metres. A probe 1000 times
  too large, or invisible because it is 1000 times too small, is this.
- **Mesh origin.** The STL's own origin and orientation are whatever the CAD had. Decide where
  each part's origin is (for example the mounting face) and export consistently.
- **Collision geometry should be simple.** Use boxes and cylinders for collision, the detailed
  mesh only for display. Planning gets faster and more robust.
- **Slicer shows only STL or OBJ visuals**, and reported errors for links with no visual in your
  log. Those messages are noise, not a fault.
- **The simulated robot is too good.** MuJoCo has no joint friction model matching the real arm,
  no cable drag, no calibration error. Numbers measured in simulation are a lower bound.

## 6. MoveIt2

- **What it does and does not do.** MoveIt2 plans collision-free joint trajectories and sends
  them to a trajectory controller. It does not control force and does not know about contact.
- **Planning group tip.** Plan for `probe_tip`, not the flange. This is set in the SRDF.
- **Cartesian paths can be partial.** `computeCartesianPath` returns a fraction between 0 and 1:
  how much of the path it could follow. Anything below about 0.99 means the path was cut short.
  Never execute a partial path as if it were complete; the executor refuses below a threshold.
- **Wrist flips and joint limits.** A path that looks smooth in space can require a joint to swing
  around. The Panda's seven joints help, but its limits (especially joint 4 and joint 6) are
  tight. Where you place the phantom relative to the robot base changes how much of the surface
  is scannable. Find a good placement in simulation before fixing anything to the table.
- **Singularities.** Near a straight arm or aligned wrist axes, small tip motions need huge joint
  speeds. Keep the working area in the comfortable middle of the workspace.
- **Collision with the thing you want to touch.** If the phantom is a collision object, MoveIt2
  refuses to touch it. For hovering paths (Phase 3) keep it as an obstacle. For contact scanning
  the scan segment is not run through MoveIt2 at all (decision D7).
- **Start slow.** Set velocity and acceleration scaling to 0.1 while developing, in simulation
  too, so the habit exists before the real robot.
- **Trajectory start mismatch.** If the robot is not where the plan started (it moved in
  between), execution is rejected. Plan and execute close together.
- **The torque-based trajectory controller needs sensible gains** in simulation. If the simulated
  arm sags, oscillates or lags, that is controller tuning, not MoveIt2.

## 7. Scan path and the camera

- **Normals are noisy.** A normal estimated from few neighbours on a noisy depth image jitters,
  and the probe orientation jitters with it. Use a neighbourhood of 1 to 2 cm and smooth along the path.
- **Normal direction is ambiguous.** The maths gives a line, not a direction. Orient every normal
  towards the camera position (or upwards) before using it, or half your poses point out of the object.
- **Free rotation about the probe axis.** The normal fixes where the probe points; the rotation
  about that axis is yours to choose. We tie it to the travel direction so the image plane is
  either along or across the motion. Decide which you want for reconstruction: sweeping
  **across** the image plane (image plane perpendicular to travel) fills a volume; moving
  **along** it only widens one slice. This is a parameter in the path generator.
- **Depth cameras have a minimum distance.** Many cannot see closer than 20 to 30 cm. A camera on
  the flange next to the probe must look at the surface from far enough away, so the capture pose
  is different from the scanning pose. This shapes the mount (section 12).
- **Depth cameras fail on shiny, dark, transparent and wet surfaces.** Ultrasound gel is wet and
  shiny. Capture the surface **before** applying gel.
- **The surface moves.** A phantom stays put; an arm does not. Anything scanned on a person is
  valid only as long as they have not moved since the capture.
- **Camera-to-robot calibration errors appear as a path that floats above or digs into the
  surface.** Task P6-T4 measures this by touching known points; do not skip it.
- **Realtime kernel.** Camera drivers that need kernel modules may not build on this kernel
  **(verify)**. A user-space backend of the driver usually exists as a fallback.

## 8. 3D Slicer and SlicerROS2

- **Slicer has its own Python (3.9).** Packages installed for the system Python are invisible to
  it. Install into Slicer's Python from the Slicer Python console with `slicer.util.pip_install("name")`.
  `rclpy` cannot be used there at all.
- **Coordinates.** Slicer works in millimetres, in the RAS patient coordinate system. ROS works in
  metres. Find out exactly what SlicerROS2 converts (task P1-T6) and write it in
  `ARCHITECTURE.md`. A reconstruction that is 1000 times too small or mirrored is this.
- **One scale for the whole robot.** Symptom: the robot is tiny (or huge) in Slicer but fine in
  RViz. Cause: SlicerROS2 (`e7bf7fe`) takes the `scale` of the meshes, keeps only the last one, and
  applies it to every link. So all robot meshes, including our probe, mount and camera STLs, must be
  in metres with no `scale` attribute. Details in `ARCHITECTURE.md` section 6.
- **Array index order.** `slicer.util.arrayFromVolume` returns arrays indexed (k, j, i), that is
  slice, row, column. The reverse of what you expect.
- **A volume's geometry is its own matrix.** Each volume has an IJK-to-RAS matrix (spacing,
  origin, direction) and may additionally sit under a transform node. Know which of the two
  carries what. For the ultrasound image: pixel spacing either in the volume or in `ImageToProbe`, not both.
- **Transforms are a hierarchy.** Putting node A "under" transform T moves A by T. Chains are
  built by nesting. To get a point's position in the base frame, ask Slicer for the transform
  from the node to world rather than multiplying by hand.
- **Developing a module.** Enable developer mode in the application settings; every scripted
  module then shows "Reload" and "Reload and Test" buttons, so you do not restart Slicer for each
  change. Create the module with the Extension Wizard so the structure (module, widget, logic,
  test classes) is standard. Put all real work in the logic class so it can be tested without the GUI.
- **Adding module paths.** Application settings, Modules, "Additional module paths". This is how
  your module and the SlicerIGSIO/SlicerIGT builds get loaded.
- **Save scenes as `.mrb`** for each experiment; one file holds everything.
- **Slicer and ROS in one process.** Slicer was started from a terminal with ROS sourced, and
  SlicerROS2 spins the ROS node on a timer. If Slicer is busy (long reconstruction), ROS data is
  not processed meanwhile. Record first, reconstruct afterwards.
- **Rebuilding SlicerROS2 writes into the Slicer build folder**, and its source has an uncommitted
  edit that makes it compile on Humble. Never reset or pull that repository without saving the edit.
- **Help.** The Slicer forum (discourse.slicer.org) is active and the SlicerROS2 authors answer
  there. Search before asking; post exact versions when you ask.

**The circularity trap.** In Phase 4 the synthetic image is cut from a known volume using the
probe pose, and the reconstruction puts it back using the same pose. If the pose is wrong, both
steps are wrong in the same way and the result still looks perfect. So this experiment proves the
recording and reconstruction machinery, and **not** that the transforms are right. Transform
correctness is proven separately: by numbers (P1-T6), and later with real images of real objects.
The perturbation experiment in P4-T7 turns this weakness into a result: it shows how much
reconstruction quality drops per millimetre or degree of calibration error, which tells you how
good the real calibration must be.

## 9. Ultrasound images and calibration

- **Three calibrations.** (1) Where the probe is on the flange, from the mount design, checked by
  P5-T5. (2) Where the image is on the probe and how big a pixel is (`ImageToProbe`), by
  experiment. (3) The delay between image and pose streams. Each has its own error; they add up.
- **Fix the machine settings.** Pixel size changes with the depth setting. Choose depth, zoom and
  focus once, write them down, photograph the screen, and never change them without recalibrating.
- **Crop the image.** A grabbed screen contains menus and text around the actual image. Define
  the crop rectangle once and store it with the calibration.
- **Speed of sound.** The machine assumes about 1540 m/s (tissue). Water at room temperature is
  about 1480 m/s, so depths measured in a cold water bath are a few percent off. Use warm water
  (near 37 °C is close to tissue speed **(verify the exact figure)**) or a proper phantom material, or correct for it.
- **Spatial calibration idea.** Image a small fixed target from many different probe poses. Its
  position in the base frame is the same every time; the only unknowns are `ImageToProbe` and the
  target position. Many poses with very different orientations make the solution well determined;
  poses that are all similar do not.
- **Report the calibration residual.** How far apart do the reconstructed positions of the same
  target lie? That number, in millimetres, belongs in the thesis.
- **Timing.** If images arrive 80 ms after the pose they belong to and the probe moves at
  20 mm/s, each image is placed 1.6 mm off. Either measure the delay, or stop the robot for each
  image (decision D11). Slow scanning reduces the error in any case.
- **Coupling.** No gel or water means no image. Air gaps look like shadows. On the phantom, use
  plenty of gel; check what the phantom's manufacturer allows.
- **Reconstruction settings.** Output voxel size should be near the pixel size or a bit larger.
  Gaps between slices appear as holes; scan slower or enable hole filling. Record first, then try
  settings on the recording, rather than rescanning.

## 10. Contact and force

- **Why not position control.** A position-controlled robot pushes as hard as needed to reach its
  target. If the target is 2 mm inside a rigid object, the force rises until the robot's safety
  reflex stops it. Impedance control makes the robot behave like a spring instead: force is
  proportional to how far it is held away from its target.
- **Choosing stiffness.** With a stiffness of 500 N/m along the probe axis, a target 10 mm below
  the surface gives about 5 N. Soft along the probe axis, stiffer sideways so the path is kept.
- **The force is an estimate.** The Panda has no force sensor at the flange; it estimates external
  force from joint torques. Accuracy is in the order of a newton or a few **(verify with a scale)**,
  worse near singular poses. Put this limitation in the thesis.
- **Payload must be right.** The estimate subtracts the known weight of what is mounted. If the
  mass or centre of mass of mount, probe and camera is wrong, the robot believes it is being
  pushed when it is not, drifts in hand-guiding, and your force readings are biased. Cables
  hanging on the probe pull on it too: route them so their pull is small and constant.
- **Friction.** Dragging a probe sideways on a surface adds sideways force. Gel lowers it. Slow
  speeds (5 to 20 mm/s) are normal for robotic scanning.
- **Order of experiments.** Simulation, then the real robot against a rigid dummy block with a
  kitchen scale under it, then the phantom, and only then anything else. Never skip a step.

## 11. The real robot and safety

**Every time**
- The stop button is in your hand or within reach **before** enabling motion. Know which button
  does what (user stop against emergency stop).
- Nobody inside the reach of the arm while it moves under program control.
- First run of anything new: reduced speed, hand on the stop, eyes on the robot and not on the screen.
- Run a new motion in simulation first, the same commands, the same launch arguments.
- Do not work alone in the lab with the robot moving. Follow the lab's own rules; they override this guide.

**Setting up**
- The control PC connects to the robot's control box by a direct network cable with fixed
  addresses. The control interface must be activated in the robot's web interface (Desk), and the
  brakes released.
- libfranka 0.9.2 fits Panda system version 4.2.x. A robot updated beyond that would need another
  libfranka and possibly another repo version; check the version in Desk (lab checklist A).
- Set the end-effector mass, centre of mass and inertia in Desk (or through the provided
  services) whenever what is mounted changes.
- The 1 kHz control loop is sensitive to PC load. Communication errors while Slicer, RViz or a
  camera driver run point to an overloaded PC: close what is not needed, set the CPU governor to
  performance, or move Slicer to a second PC (decision D12).
- After a reflex stop, the error must be cleared (error recovery) before moving again. Find out
  **why** it stopped before clearing it; the reason is in the robot state.

**Scanning a person (the arm demonstration)**
- This is a different category from a phantom. It needs your supervisor's explicit approval and
  very likely your institution's ethics or safety procedure. Ask early; such approvals take weeks.
- The Panda is a collaborative research robot, not a medical device.
- Preconditions: the safety layer (P8-T4) proven with deliberate faults; force limit low; speed
  low; the scanned person holds a stop button and can pull the arm away freely; the arm is never
  trapped between probe and a hard surface without an escape direction.
- Plan the thesis so that it is complete without this demonstration.

## 12. Designing the mount

You are designing this before the first plan phase needs it, so here is what the rest of the
project needs from it:

- **Rigid and repeatable.** Every calibration assumes the probe sits at exactly the same place
  each time it is mounted. Use locating features (pins, a keyed shape), not just clamping
  friction. If the probe can be reinserted 1 mm differently, your reconstruction is 1 mm worse.
- **Known geometry.** Design it so the transducer face centre and the probe axis have defined
  positions relative to the flange face. Those numbers go straight into the xacro.
- **Flange pattern.** Take the hole pattern and centring diameter from the Panda's manual or by
  measuring the flange (lab checklist A). Do not guess.
- **Probe axis.** Easiest for planning: probe axis parallel to the flange axis, or at a known
  fixed angle. An offset from the flange axis is fine if known.
- **Camera placement.** The camera needs a clear view of the surface at its working distance,
  with the probe not blocking the view more than necessary. It must not hit the object while the
  probe scans. Check the camera's minimum distance and field of view first (checklist C).
- **Weight.** The Panda's payload is 3 kg including everything bolted on and the cable pull. Keep
  the mount light and the centre of mass close to the flange axis: less torque on the wrist,
  better force estimates.
- **Cables.** Give the probe cable and the camera cable a strain relief on the mount and a route
  along the arm that neither pulls nor snags over the whole working range. Check joint 7's full rotation.
- **Do not damage the probe.** Clamp on the housing, with a soft liner, never on the cable or the
  transducer face. Ask the probe's owner before clamping anything.
- **Calibration aids.** A flat reference face or a divot on the mount, at a known position, makes
  checking the tool tip and the camera calibration much easier later.
- **Measure what you built.** A printed part differs from its CAD. Measure the real one and put
  the real numbers in the model, or calibrate.
- **Give me the files.** Export the mount, probe and camera as separate STL files with a
  documented origin, plus the mass of each part from a scale.

## 13. Thesis habits

- **Start writing in January.** Method chapters can be written as each phase finishes, while you
  still remember why things were done. `DECISIONS.md` is already half of your design chapter.
- **Keep a lab notebook** (a dated text file in the repo is enough): what you tried, what
  happened, the numbers.
- **Every experiment is reproducible.** Store with each run: the git commit, the parameter
  files, a rosbag of the relevant topics, the Slicer scene, the machine settings, a photo of the setup.
- **Record rosbags generously.** A bag lets you redo analysis without the robot. Lab time is scarce; replay is free.
- **Take pictures and screenshots as you go.** The figure you need in April is the setup you dismantled in February.
- **Measure before improving.** Each phase ends with a number. Without the "before" number an improvement cannot be shown.
- **State limitations plainly.** Examiners respect a measured limitation far more than an unexplained good-looking result.
- **Agree the evaluation with your supervisor early** (the list in `ROADMAP.md`), so the last month has no surprises.
- **Commit often, push daily.** The docs and plan are only in this repository; an unpushed laptop is a single point of failure.

## 14. Known problems on this machine

Found during setup on 2026-11-15 to 17. Add to this list as you go.

- **Slicer: "could not find the transform … panda_link0 … does not exist" (explained in P0-T8,
  2026-11-24).** Not a failure. Slicer already has the robot model but has not yet received a `/tf`
  message for the moving joints (links 0 to 7 and the fingers; fixed joints come on the latched
  `/tf_static` and are there at once). Slicer retries every 20 ms and prints the lines on every try.
  - Robot added to a running sim: no lines, or a few rounds.
  - Robot added before or while the sim starts: many rounds (half a second or more).
  - Either way it stops by itself and the robot then follows. Launch file (sim only or MoveIt),
    start order, sim time and the old base `~/ros2_ws_2` make no difference (five runs, see
    `docs/project/briefs/P0-T8.md`).
  - Tip: add the robot in Slicer after MuJoCo or RViz shows the arm.
  - Only lines that **never stop** are a real problem: check that the sim runs and `ros2 topic hz /tf`.
  - Also harmless on every robot load: "no visual vector available for link 8 / 12" (`panda_link8`
    and `panda_hand_tcp` have no mesh), two "A FileName must be specified" from `vtkSTLReader`, and
    "parent transform cannot be self" (the root link refused as its own parent).
- **Slicer's log is in `/tmp/Slicer-daniel/`** and is deleted on reboot. Copy it out when a run
  matters; `~/.ros/log/SlicerApp-real_*.log` stays empty.
- **Decimal comma breaks ROS parameters in Qt programs.** The desktop's number format is `hr_HR`.
  RViz (and Slicer) apply it, and ROS 2 then reads YAML numbers like `0.05` as text; MoveIt's RViz
  display failed to load the robot. `~/start_env.sh` now sets `LC_NUMERIC=en_US.UTF-8` (P0-T3).
  A terminal not started from `start_env.sh` still has the problem.
- **`ros2 node list` shows nothing although nodes run:** the ROS 2 daemon is stale. Use
  `--no-daemon`, or `ros2 daemon stop` and try again. `start_slicer` uses `--no-daemon` since 2026-11-21.
- **Nodes in one terminal are invisible to another (empty rqt_graph, Slicer cannot find
  `/robot_state_publisher`):** the terminals differ in `ROS_LOCALHOST_ONLY`. `~/start_env.sh` sets
  it to 1 since 2026-11-21; start every ROS program from a terminal that sourced it.
- **`sim_moveit.launch.py` works but has known rough edges** (P0-T3, 2026-11-21):
  - Start MuJoCo with Space: the simulation starts paused, and execution only works when it runs.
  - The arm is red in RViz: since P0-T4 the visual meshes are STL without `<material>`, and RViz
    paints uncoloured meshes with its default `RVIZ/ShadedRed`. Cosmetic.
  - "Maybe failed to update robot state, time diff: ~1.8e9 s" in RViz: the MuJoCo side uses sim
    time, `move_group` and RViz use wall time. Execution still succeeded. To fix in our own launch (P2-T2).
  - The MoveIt gripper controller points at `panda_gripper/gripper_action`, but the sim server is
    `panda_gripper_sim_node`. Gripper moves from MoveIt will fail. Not needed: the probe replaces it.
  - The `class_loader` "SEVERE WARNING ... InteractiveMarkerDisplay" from RViz is harmless: MoveIt's
    RViz plugin links RViz's default plugins directly. Nothing to do on our side.
- **SlicerROS2 is a version meant for ROS 2 Jazzy and Slicer 5.8**, running here on Humble and
  Slicer 5.6.2 thanks to a local edit. A feature that fails for no visible reason may simply not work in this combination.
- **Slicer was built with its own bzip2** although the SlicerROS2 instructions ask for the system one. It built anyway; remember this if a strange load error appears.
- **libfranka 0.9.2 is installed twice** (`/usr` and `/usr/local`). Same version, so harmless today; confusing if one is ever updated.
- **`~/.bashrc` puts MuJoCo 3.2.0 (`~/Libraries/mujoco-3.2.0`), Eigen 3.3.9 (`/opt/eigen-3.3.9`) and
  `/usr/local` first** on the search paths. Builds pick those over system versions. MuJoCo 3.1.6 in
  `/usr/local` is only for the old workspace `~/ros2_ws_2`; rebuilding that workspace now picks 3.2.0 and may fail.
- **Claude Code's shell does not read `~/.bashrc`.** It sources `~/start_env.sh` instead, which sets
  the same paths. Change both files together if a path changes.
- **`rm -rf` and `sudo` are blocked for the assistant** by `.claude/settings.json`. That is intended; you run those yourself.
- **The Claude Code setup is in the repo** (`CLAUDE.md`, `.claude/`). Start sessions from the repo
  folder `~/ros2_ws_3/src/multi_franka_arm_ros2`, not from `~/ros2_ws_3` or the old
  `~/ros2_ws_2/...`, or the rules and the subagent are not loaded (or the outdated ones are).
