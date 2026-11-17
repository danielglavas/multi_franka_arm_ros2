# Lab checklist

Fill this in at the lab. Each answer unlocks a decision or a phase (named in brackets). Write the
answer under the question; add photos to `docs/project/lab_photos/` if useful.

## A. Robot (unlocks Phase 0 decision D1, Phase 2 R0, Phase 5)

- [ ] Panda system version shown in Desk (needed: compatible with libfranka 0.9.2, i.e. 4.2.x):
- [ ] Robot IP address for the control interface, and the PC's network setup:
- [ ] Is the control interface (FCI) feature available and activated?
- [ ] Is this PC (realtime kernel) the one that will control the real robot?
- [ ] Flange: hole pattern, hole size, centring diameter, locating pin (measure or take from the manual):
- [ ] Is the gripper mounted now, and may it be removed?
- [ ] Table: size, height of the surface relative to the robot base, where the phantom can stand:
- [ ] Where are the user stop and emergency stop, and what does each do?
- [ ] Lab rules for working with the robot (alone or not, hours, sign-off):
- [ ] Who else uses ROS 2 on the lab network (need for a private `ROS_DOMAIN_ID`)?

## B. Probe (unlocks Phase 1 dimensions, Phase 5)

- [ ] Probe manufacturer and model:
- [ ] Type (linear, convex) and footprint of the transducer face (width, thickness, curvature):
- [ ] Outer dimensions of the housing; photos from all sides with a ruler:
- [ ] Where may it be clamped? Any restrictions from its owner?
- [ ] Mass of the probe (scale), and how the cable leaves it:
- [ ] Is a 3D model of the probe available from the manufacturer or the lab?

## C. 3D camera (unlocks D10, the mount design, Phase 6)

- [ ] Exact model (label on the camera):
- [ ] Minimum and ideal working distance, field of view (datasheet):
- [ ] Connection (USB 3 needed?) and cable length to the PC along the arm:
- [ ] Mounting holes on the camera:
- [ ] Mass:
- [ ] Does its driver run on this PC? (Try: plug in, run the vendor viewer or the ROS driver.)
- [ ] Is a calibration board (checkerboard, ChArUco) available in the lab?

## D. Ultrasound machine (unlocks D6, D11, D13, Phase 7)

- [ ] Manufacturer and model of the machine:
- [ ] Video outputs on the machine (HDMI, DVI, VGA, S-Video, none):
- [ ] Is there a research interface, SDK or network streaming option? Which operating system does it need?
- [ ] Is a frame grabber available in the lab? Model:
- [ ] Is the machine or the frame grabber listed as supported by the PLUS toolkit?
- [ ] Has anyone in the lab already streamed images from it into Slicer or a PC? How?
- [ ] Can images or clips be saved on the machine and exported (format)?
- [ ] Typical imaging depth for the phantom, and the pixel size at that depth if shown:
- [ ] Is there a foot switch or trigger input (useful for stop-and-go acquisition)?
- [ ] Calibration phantoms or tools available (wire phantom, point target, water bath):

## E. Phantom and setup (unlocks Phases 5 to 9)

- [ ] Abdomen phantom: manufacturer, model, size, what structures are inside:
- [ ] Is ground truth available (datasheet dimensions, a CT or MR scan of the phantom)?
- [ ] What may be put on it (gel, water)? Cleaning rules?
- [ ] Can it be fixed to the table so it does not slide under the probe?
- [ ] Is a kitchen scale or force sensor available to check contact force?

## F. People and approvals

- [ ] Supervisor's view on the evaluation list in `ROADMAP.md`:
- [ ] What approval is needed to scan a person's arm, and how long does it take?
- [ ] Is a second PC available for Slicer if the control PC is overloaded (D12)?
- [ ] Access to a 3D printer or workshop for the mount, and its turnaround time:
