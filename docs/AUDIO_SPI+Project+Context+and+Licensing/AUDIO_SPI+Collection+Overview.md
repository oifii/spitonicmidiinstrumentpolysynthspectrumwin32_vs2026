# AUDIO_SPI Project Context and Licensing

## AUDIO_SPI Collection Overview 🎹

AUDIO_SPI is a family of open source Windows audio applications. This repository is a member of that collection, focused on MIDI-driven polyphonic synthesis and spectral tools. The collection leverages well-known libraries to ease porting and experimentation.

- **Collection Name:** AUDIO_SPI software collection
- **Target Platform:** Windows desktop
- **Core Technologies:**
- Win32 API
- PortAudio / PortMidi
- Tonic synthesis library
- **Repository Purpose:**
- MIDI-driven polyphonic synthesizer
- Spectrum-related audio analysis tools

## Historical Background

Stephane Poirier founded AUDIO_SPI in Montreal circa 2010. He aimed to experiment with audio processing and learn software-based music creation. Over time, composer Carl Poirier used several AUDIO_SPI apps to prototype an audio OS concept called XAOS. Packaging sometimes reflects XAOS requirements.

## Goals and Use Cases

- **Experimentation:** Rapidly prototype synthesis and audio tools.
- **Portability:** Use popular open source libraries to simplify cross-platform work.
- **Education:** Understand the process of creating music via software.
- **Research:** Support XAOS, an experimental audio operating system.

## Project Metadata

| Field | Details |
| --- | --- |
| **Founder** | Stephane Poirier |
| **Origin** | Montreal, QC, Canada |
| **Start Year** | 2010 |
| **Website** | https://audiospi.com |
| **Support Forum** | https://groups.google.com/forum/#!forum/audio_spi-users |
| **License** | GNU General Public License v3 or later (GPL-3.0+) |


## Licensing 📜

All AUDIO_SPI applications, including this synthesizer and spectrum tool, are distributed under the **GNU General Public License version 3 or later**. This ensures free usage, modification, and redistribution under the same license terms.

- **License Type:** Copyleft (GPL-3.0+)
- **Warranty:** No warranty of merchantability or fitness for a particular purpose.
- **Source Availability:** Full source is provided; recipients may modify under GPL terms.

```c
// Example from Resource.h showing licensing header
/* 
 * Copyright (c) 2015-2016 Stephane Poirier
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
```

The same header pattern appears across source files (e.g., `spitonicsynths.cpp`), ensuring consistent GPL-3.0+ coverage throughout the codebase.

## Dependencies and Relationships

- **Win32 API:** Handles windowing, GUI, and core OS integration.
- **PortAudio / PortMidi:** Manages audio streaming and MIDI I/O across platforms.
- **Tonic Library:** Provides synthesis primitives, envelopes, filters, and more.
- **Other AUDIO_SPI Apps:** Shares conventions and build processes with sister projects in the collection.

These dependencies highlight the project’s emphasis on leveraging established libraries for rapid development and cross-platform potential.