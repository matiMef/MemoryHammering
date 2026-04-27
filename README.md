# Memory Hammering - DRAM Bit Flip Analysis

This project explores the phenomenon of Rowhammer, a hardware-level vulnerability in modern DRAM. It focuses on the practical implementation of memory "hammering" techniques to observe and analyze unintended bit flips caused by electromagnetic interference between adjacent memory cells.

## Project Overview
The primary goal of this repository is to demonstrate how frequent, repeated access to specific memory rows can bypass traditional software-level memory protection. By inducing bit flips, the project highlights critical security implications for system integrity and isolation.

## Technical Details
* **Target Mechanism**: Leveraging the "Rowhammer" effect by performing rapid, uncached memory reads (using `clflush` or similar instructions) to stress DRAM rows.
* **Memory Management**: Implementation of precise memory mapping to identify adjacent rows and optimize the chance of a successful bit flip.
* **Performance Monitoring**: Tools to track the frequency of accesses and the time required to induce state changes in the hardware.

## Key Features
* **Double-Sided Hammering**: Implementation of optimized patterns that target a specific row from both adjacent neighbors simultaneously.
* **Automated Testing**: Scripts to scan memory regions for vulnerable cells and document the addresses of successful flips.
* **Data Logging**: Detailed reporting of bit-flip locations and the patterns used to trigger them.

## Safety and Ethics Warning
This project is intended for educational and research purposes only. "Memory Hammering" involves stressing hardware components beyond their standard operating conditions.
* Use this software only on hardware you own.
* Be aware that repeated hammering can lead to system instability or data corruption.
* The authors are not responsible for any hardware damage resulting from the use of these tools.

## Requirements
* A Linux-based environment (recommended for direct memory access control).
* C/C++ compiler with support for low-level memory intrinsics.
* Hardware susceptible to Rowhammer (typically non-ECC DDR3 or DDR4 memory).
