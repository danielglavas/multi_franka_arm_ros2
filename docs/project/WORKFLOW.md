# Workflow: how the plan is executed

Three roles work on this project. This file says who does what, so that a session started by any
of them can continue where the last one stopped.

| Role | Who | Does |
|---|---|---|
| Owner and student | Daniel | decides, does all lab and `sudo` work, runs real-robot commands, learns |
| Planner and reviewer | Opus | picks the next task, writes the task brief, reviews the result, updates `PLAN.md`, teaches |
| Implementer | Sonnet | writes code and tests from a task brief, nothing beyond it |

`CLAUDE.md` rules apply to every role: explain before doing, comment all code, name alternatives,
propose before changing files.

## Running the two models

- Simplest: one session with Opus; it hands a task brief to a Sonnet subagent and reviews what comes back.
- Alternative: switch the model in the session (`/model`) between planning and coding.
- Some Claude Code versions offer a combined mode that plans with Opus and executes with Sonnet;
  check `/model` for it.
- Whatever the mechanism, the hand-over is the **task brief**. If the brief is not written down,
  the hand-over did not happen.

## Start of every session (Opus)

1. Read `CLAUDE.md`, then `docs/PLAN.md`. Read `ARCHITECTURE.md` and `DECISIONS.md` when the task touches them.
2. Check `git status` and the current branch. Uncommitted work from a previous session is
   finished or reported first, never discarded.
3. Tell Daniel in a few sentences: where the plan stands, what the next task is, what it needs from him.
4. If the next task depends on an open decision or missing lab information, say so and pick
   another task that does not, instead of guessing.

## Life of a task

1. **Select.** Next unblocked task in `PLAN.md`; mark it `[~]`.
2. **Brief.** Opus writes the brief from the template below. For code tasks Opus first reads the
   existing code the task touches; briefs never describe files from memory.
3. **Explain.** Opus tells Daniel what will be built, why, and what he will learn from it. Daniel approves.
4. **Implement.** Sonnet works from the brief only. If something in the brief is wrong or missing,
   Sonnet stops and reports; it does not redesign.
5. **Check.** Build, run the tests, run the "Done when" check for real. A check that needs the
   simulator or Slicer is done by Daniel with exact commands given to him.
6. **Review.** Opus reads the diff against the review checklist.
7. **Teach.** Opus walks Daniel through the code: what each part does and one thing to try changing.
8. **Close.** Update `PLAN.md` (`[x]`, date, measured numbers), `ARCHITECTURE.md` if an interface
   changed, `DECISIONS.md` if something was decided. Commit only when Daniel asks; one task per commit.

## Rules for Sonnet

- Do exactly the brief. No extra features, no renames outside the listed files, no new dependencies without them being in the brief.
- Never change names of topics, frames, services or parameters; they come from `ARCHITECTURE.md`.
- Every file starts with a comment saying what it is for. Every function has a docstring. Names describe purpose.
- Write the tests named in the brief; do not weaken or skip a test to make it pass.
- Never run `ros2 launch`, `ros2 run`, anything touching the real robot, or `sudo`. Give the command to Daniel.
- Finish with a short report: files changed, test results with real output, anything not done, anything that looked wrong in the brief.

## Rules for editing the plan (Opus)

- Tasks may be split, reordered or re-dated when reality requires it. Write one line in the change log each time.
- Checkpoints (R0, M-A, M-B, M-C, feature freeze) and the "never cut" list change only with Daniel's agreement.
- A phase is not started just because its date arrived; its inputs must exist.
- When a task takes more than twice its expected effort, stop and tell Daniel with options (simplify, change approach, cut).
- Do not turn an outline task of Phases 5 to 9 into a brief until the lab information it names is in `LAB_CHECKLIST.md`.

## Review checklist (Opus)

- [ ] Does what the brief asked, and nothing else.
- [ ] Builds without new warnings; tests pass; baseline has no new failures.
- [ ] Tests would fail if the code were wrong (not only "runs without error").
- [ ] Names and interfaces match `ARCHITECTURE.md`.
- [ ] Comments explain purpose and non-obvious maths; no numbers hard-coded that should be parameters.
- [ ] Units and frames are stated wherever a pose, length or angle crosses a function boundary.
- [ ] Nothing can move the robot without an explicit call from a person.
- [ ] Failure paths give a clear message (missing transform, empty cloud, unreachable pose).

## When to stop and ask Daniel

- Anything on the real robot, anything needing `sudo`, any `git push`.
- A decision in `DECISIONS.md` would have to be assumed.
- A change inside a `franka_*` package or in `~/ros2_ws` (the Slicer module).
- A test fails and the fix is not obvious within the task.
- Measured results contradict the plan's assumptions.

---

## Task brief template

```markdown
# Brief <task ID>: <title>

## Goal
One or two sentences: what exists afterwards and why the project needs it.

## Read first
Files (with paths) the implementer must read before writing anything.

## Create / change
Exact file paths. For each: what it contains.

## Interface
Names, types, units and frames of every topic, service, parameter, function signature.
Copied from ARCHITECTURE.md, not invented.

## Behaviour
What it does, step by step, including what happens on bad input.

## Tests
Each test: what it checks and the pass condition, with numbers.

## Done when
Commands to run and the expected result.

## Out of scope
What must not be done in this task.

## For Daniel
What this task teaches, and one small experiment to try afterwards.
```

---

## Example brief 1 (task P1-T5)

# Brief P1-T5: probe pose publisher node

## Goal
A small Python node that publishes the pose of the probe tip in the robot base frame. It is
Daniel's first node and gives a simple topic for logging the probe pose during experiments.

## Read first
- `docs/project/ARCHITECTURE.md`, sections 5 and 6.
- The package skeleton `ultrasound/ultrasound_scan_planning/` from task P0-T7.

## Create / change
- `ultrasound/ultrasound_scan_planning/ultrasound_scan_planning/probe_pose_publisher.py`: the node.
- `ultrasound/ultrasound_scan_planning/ultrasound_scan_planning/pose_conversion.py`: one pure
  function converting a `TransformStamped` to a `PoseStamped`, no node code.
- `ultrasound/ultrasound_scan_planning/test/test_pose_conversion.py`: tests.
- `setup.py`: add the console script `probe_pose_publisher`.
- `package.xml`: add `rclpy`, `tf2_ros`, `geometry_msgs` if missing.

## Interface
- Publishes `/scan/probe_pose`, `geometry_msgs/PoseStamped`, header frame = base frame, metres.
- Parameters: `base_frame` (string, default `panda_link0`), `probe_tip_frame` (string, default
  `probe_tip`), `publish_rate_hz` (double, default 30.0).

## Behaviour
- A timer at `publish_rate_hz` looks up the latest transform from `base_frame` to `probe_tip_frame`.
- If the transform is available, convert and publish it with the transform's own timestamp.
- If it is not available, do not publish; log a warning at most once every 5 seconds naming both frames.
- Rate of zero or below: log an error and exit.

## Tests
- Conversion keeps translation and rotation values and copies the timestamp and frame.
- Conversion of an identity transform gives position (0, 0, 0) and quaternion (0, 0, 0, 1).

## Done when
- `colcon build --packages-select ultrasound_scan_planning` and
  `colcon test --packages-select ultrasound_scan_planning` succeed.
- With the simulation from P1-T4 running, `ros2 topic echo /scan/probe_pose` shows a pose that
  changes when the robot moves (Daniel runs this).

## Out of scope
Custom messages, services, launch file changes, anything Slicer-side.

## For Daniel
Teaches: node, parameter, timer, tf listener, publisher, separating logic from ROS for testing.
Try: change `publish_rate_hz` from the command line and confirm it with `ros2 topic hz`.

---

## Example brief 2 (task P3-T3)

# Brief P3-T3: path generation library

## Goal
Turn a few clicked points on a surface into an ordered list of probe poses that follow the
surface. This is the core planning step; the ROS node of P3-T4 only wraps it.

## Read first
- `docs/project/ARCHITECTURE.md`, section 6 (definition of the `probe_tip` axes).
- `surface_processing.py` from P3-T2 (point and normal arrays, nearest-neighbour search).

## Create / change
- `ultrasound/ultrasound_scan_planning/ultrasound_scan_planning/path_generation.py`
- `ultrasound/ultrasound_scan_planning/test/test_path_generation.py`
No ROS imports in either file. numpy and scipy only.

## Interface
`generate_scan_poses(surface_points, surface_normals, waypoints, point_spacing_m,
surface_offset_m, approach_distance_m) -> list of (position_xyz, rotation_matrix_3x3)`

- `surface_points` (N, 3) and `surface_normals` (N, 3, unit length, pointing away from the
  object), in the base frame, metres.
- `waypoints` (M, 3), M at least 2, the clicked points in order.
- `surface_offset_m`: positive lifts the path off the surface, negative presses into it.
- Returned poses are `probe_tip` in the base frame: z axis along the inward normal, x axis along
  the direction of travel projected onto the surface, y completing a right-handed frame.
- The first and last returned poses are the approach and retreat poses, lifted by `approach_distance_m`.

## Behaviour
1. Snap each waypoint to its nearest surface point.
2. Between consecutive waypoints, step in straight lines no longer than `point_spacing_m` and snap
   each intermediate point to the surface.
3. Resample so neighbouring path points are `point_spacing_m` apart along the path.
4. Take the normal at each point as the average of its neighbours' normals, to reduce noise.
5. Build each rotation from the inward normal and the travel direction; at the last point reuse the previous travel direction.
6. Keep orientation continuous: the x axis must never flip by more than 90 degrees between neighbours.
7. Raise `ValueError` with a clear message for: fewer than two waypoints, empty surface, a waypoint
   further than 5 cm from any surface point, spacing not positive.

## Tests
- Plane z = 0, straight line: all positions at z = offset within 1e-6 m; z axis is (0, 0, -1); x axis along the line.
- Cylinder of radius 0.04 m, path around the circumference: positions at radius + offset within
  1 mm; z axis points at the cylinder axis within 5 degrees.
- Same cylinder with 1 mm noise on points: axis error still within 5 degrees.
- Every returned rotation matrix is orthonormal with determinant +1.
- No x-axis flip between neighbours on an L-shaped waypoint list.
- Each error case raises `ValueError`.

## Done when
`colcon test --packages-select ultrasound_scan_planning` passes with these tests included.

## Out of scope
ROS messages, reachability, collision checking, speed along the path.

## For Daniel
Teaches: building a coordinate frame from a normal and a direction, why orientation continuity
matters for a robot wrist, testing geometry against shapes with known answers.
Try: plot the returned positions and axes for the cylinder with matplotlib.
