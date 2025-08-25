DRV2624 Waveform Sequencer and RAM Operations
This document explains how the DRV2624 haptic driver's waveform sequencer works, with emphasis on internal RAM operations, sequencing, and programming flow. Illustrations and diagrams from TI documentation are included where possible for clarity.
1. Overview of Sequencer Mode
The DRV2624 haptic driver includes a waveform sequencer engine that allows multiple vibration effects to be pre-programmed and executed in sequence with a single trigger. The sequencer reads waveform definitions stored in internal RAM and plays them back in the programmed order, reducing I²C traffic and host MCU burden.
2. Internal RAM Organization
The DRV2624 has 1 KB of internal RAM divided into three logical sections:
1. Revision Byte (0x0000) – Not used for playback.
2. Header Space (starting at 0x0001) – Metadata for each waveform (start address, number of bytes, repeat count).
3. Waveform Data Area – Stores voltage/time pairs that define haptic effects.
Figure 1. DRV2624 RAM Structure (from TI documentation):
[Insert RAM structure diagram here]
3. Defining a Waveform in RAM
Each waveform is described as a sequence of amplitude (voltage) and duration (time) pairs.

- Amplitude (Voltage Byte): Sets drive strength (e.g., 0x10 ≈ 25% amplitude).
- Duration (Time Byte): Expressed in multiples of 5 ms (e.g., 0x28 = 200 ms).

Steps to program:
1. Write header entry → start address, length, repeat count.
2. Write waveform data (voltage/time pairs) into RAM data section.
3. Map waveform number into sequencer registers.
Example: A 200 ms buzz at 25% amplitude → Voltage: 0x10, Time: 0x28.
4. Playing Waveforms via Sequencer
The Waveform Sequencer registers (0x04–0x0B) define up to 8 steps in playback. Each step points to a waveform number stored in RAM, or a wait instruction if MSB=1.

- Terminate code (0x00) ends the sequence.
- MSB high = wait (e.g., 0x89 = 90 ms wait).

To play a sequence:
1. Program sequencer registers with waveform numbers.
2. Write 0x01 to GO register (0x0C) → playback begins.
3. Device autonomously executes the sequence.
5. RAM Reloading Considerations
RAM contents are lost after power loss and must be reloaded on startup. Once programmed, RAM retains values until reset. Typical flow:
- At boot: Load required waveforms into RAM.
- During operation: Trigger pre-defined sequences using waveform IDs.
6. Example Waveforms
- Buzz (200 ms, 25% amplitude)
- Ramp-Up (0 → Full scale in 100 ms)
- Ramp-Down (Full scale → 0 in 100 ms)

These can be chained using sequencer registers for complex haptic effects, e.g.:
Seq1 = Ramp-up → Seq2 = Buzz → Seq3 = Ramp-down
7. Advantages of Sequencer Mode
- Offloads timing from MCU
- Reduces I²C traffic
- Reusable waveform library
- Wait instructions enable rhythm/timing control

Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

