<div align="center">

# 🛡️ Spartan-Warrior

### An LED-matrix space shooter and MP3 audio system, running on bare-metal FreeRTOS

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=black)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-RTOS-3DDC84?style=for-the-badge)
![ARM Cortex-M3](https://img.shields.io/badge/ARM-Cortex--M3-0091BD?style=for-the-badge&logo=arm&logoColor=white)
![LPC1758](https://img.shields.io/badge/board-SJOne%20(LPC1758)-red?style=for-the-badge)

**Embedded systems project · San Jose State University · Fall 2018**

[📄 Project report](http://socialledge.com/sjsu/index.php/F18:_Spartan_Warrior)

</div>

---

## ✨ Overview

Spartan-Warrior is an embedded entertainment system built on the **SJOne board** (NXP LPC1758, ARM Cortex-M3). It has two firmware builds that share one hardware and software stack:

| Firmware | What it does |
|---|---|
| 🎮 **Game Firmware** | A multi-task **space shooter** drawn on an LED matrix. Tilt the board to steer your ship, fire at the enemies, and chase a score. It has start and game-over screens and a play-again option |
| 🎵 **MP3 Firmware** | Plays audio through an SPI-connected MP3 decoder, using a `Mp3Decoder` class that wraps the decoder's registers (mode, clock, volume, bass, audio data) |

The original project goals were an embedded music system: a playlist stored on an SD card, decoded and played through a speaker, controlled remotely from a phone over Bluetooth, with the audio spectrum shown on RGB LEDs. See the report for the full story.

## 🎮 The game

Everything in the game is its own **FreeRTOS task**, which makes it a neat study in cooperative real-time design:

```mermaid
flowchart LR
    ACC["📐 Accelerometer<br/>(board tilt)"] --> MOVE["move_player_sprite"]
    MOVE --> SPR["draw_player_sprite"]
    SPR --> MAT
    SHOT["fire_a_shot"] --> HIT
    E["5 × draw_enemy tasks"] --> HIT["detect_a_hit"]
    E --> MAT
    HIT --> SCORE["score + print_score"]
    MAT["iterate_through_led_matrix_rows<br/>(scans the display)"] --> LED["💡 LED matrix"]
    START["start / game-over screens"] --> MAT
```

- 📐 **Tilt controls**: the onboard accelerometer's Y axis is read, bucketed into steps and mapped to ship movement
- 👾 **Five enemies**, each in its own task, worth between 1 and 7 points
- 💥 **Hit detection** runs as its own task
- 🖥️ **Display scanning** is done by dedicated row-iteration tasks, with separate ones for the start/end screens

## 🧱 Software architecture

The project uses the layered SJSU framework. The lower layers are the course-provided base, and the project's own work is in `L5_Application`.

```mermaid
flowchart BT
    L0["L0 · Low level<br/>startup · clocks · UART0 · LPC17xx registers"]
    L1["L1 · FreeRTOS<br/>kernel · heap options · MPU / no-MPU ports"]
    L2["L2 · Drivers<br/>ADC · CAN · I²C · SSP/SPI · UART · PWM · RTC · timers"]
    L3["L3 · Utils<br/>circular buffer · scheduler task · command handler · logger"]
    L4["L4 · IO<br/>accelerometer · light/temperature/IR sensors · LEDs · switches<br/>FatFs on SD card · nRF24L01+ wireless mesh"]
    L5["L5 · Application ⭐<br/>Game · MP3 · peripheral drivers · terminal commands"]
    L0 --> L1 --> L2 --> L3 --> L4 --> L5
```

## 📁 Repository layout

```text
Spartan-Warrior/
└── Spartan_Warrior/
    ├── L0_LowLevel/ … L4_IO/        # SJSU framework layers
    ├── L5_Application/              # ⭐ project code
    │   ├── Game Firmware/           #   LED-matrix shooter
    │   ├── MP3 Firmware/            #   Mp3Decoder, mp3_main, test song
    │   ├── Peripheral Drivers/      #   LabGPIO, GPIO interrupts, LabSPI
    │   └── source/                  #   terminal, remote, command handlers
    ├── _cgreen_test/                # host-side unit tests (Cgreen)
    ├── loader.ld  loader_mpu.ld     # linker scripts
    ├── dox.Doxyfile                 # Doxygen configuration
    └── .project  .cproject          # Eclipse CDT project
```

## 🚀 Building

The repository is an **Eclipse CDT project** for the SJOne board, so the quickest way in is:

1. `git clone https://github.com/tanmayhc7/Spartan-Warrior.git`
2. In Eclipse (with the ARM GCC toolchain installed): **File → Import → Existing Projects into Workspace** and choose the `Spartan_Warrior` folder
3. `L5_Application` holds both firmwares (Game and MP3), so make sure the one you want is the one included in the build
4. Build and flash to the SJOne board

> Full hardware setup, wiring and results are in the [project report](http://socialledge.com/sjsu/index.php/F18:_Spartan_Warrior).

## 🛠️ Tech stack

`C++ / C` · `FreeRTOS` · `LPC1758 / Cortex-M3` · `SPI` · `GPIO interrupts` · `LED-matrix driving` · `MP3 decoding` · `Accelerometer input` · `Eclipse CDT`

---

<div align="center">

Built by [Tanmay Chandavarkar](https://github.com/tanmayhc7) with the Spartan-Warrior team · Framework by Prof. Preet Kang, SJSU

</div>
