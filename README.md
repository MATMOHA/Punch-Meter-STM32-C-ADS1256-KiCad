# 🥊 STM32 Arcade Punch Force Meter

A high-speed, embedded data acquisition and audio-visual force measurement system built on an **STM32 Nucleo-F401RE** microcontroller. The system interfaces a **500kg YZC-516C S-beam load cell** via a 24-bit **ADS1256 ADC** over **SPI**, featuring real-time peak impact capture, custom noise-isolated analog power regulation, a 4-digit TM1637 score display, and dynamic tier-based audio playback.

---

## 🛠️ System Architecture & Hardware Stack

* **Microcontroller:** STM32 Nucleo-F401RE (ARM Cortex-M4 @ 84MHz)
* **Analog-to-Digital Converter:** Texas Instruments ADS1256 (24-bit Delta-Sigma ADC with SPI interface)
* **Sensor:** YZC-516C 500kg S-Beam Strain Gauge Load Cell
* **Display:** TM1637 4-Digit 7-Segment LED Display
* **Audio Module:** DFPlayer Mini MP3 Player (UART Interface @ 9600 baud)
* **Power Management:** TP4056 Li-Ion Charging Module + Boost Converter with dedicated low-noise analog power rails

---

## 🔌 Pinout & Hardware Interfacing Table

| Peripheral Module | Signal / Function | STM32 Nucleo-F401RE Pin | Notes |
| :--- | :--- | :--- | :--- |
| **ADS1256 ADC** | SPI SCLK | `PA5` | Hardware SPI1 SCLK |
| | SPI MISO (DOUT) | `PA6` | Hardware SPI1 MISO |
| | SPI MOSI (DIN) | `PA7` | Hardware SPI1 MOSI |
| | Chip Select (`CS`) | `PB6` | Active Low |
| | Data Ready (`DRDY`) | `PA9` | Hardware Interrupt / Polling |
| | Reset (`RESET`) | `PA8` | Active Low |
| **TM1637 Display** | Clock (`CLK`) | `PB10` | GPIO Output |
| | Data (`DIO`) | `PB3` | GPIO Output |
| **DFPlayer Mini** | UART TX (Module RX) | `PA2` (USART2_TX) | Requires 1kΩ inline resistor |
| | UART RX (Module TX) | `PA3` (USART2_RX) | 9600 Baud Audio Control |
| **Status LEDs** | System Ready | `PC13` | Onboard LED |

---

## ⚡ Firmware & Signal Conditioning

* **Auto-Zeroing / Tare Routine:** Performs baseline offset sampling at boot to zero ambient mechanical preload.
* **150 ms Peak Force Detection:** Tracks differential voltage swings across the bridge output during impact windows, recording max amplitude.
* **Force Calibration:** Scaled dynamic force calculations against a calibrated **40 lb reference weight**.
* **Analog Power Isolation:** Uses separate linear regulation rails for the ADS1256 analog front-end to eliminate switching noise from the boost converter and digital microcontrollers.
* **Tier-Based Audio Feedback:**
  * `Tier 1 (50 – 199 lbs)`: Light impact trigger
  * `Tier 2 (200 – 399 lbs)`: Medium impact trigger
  * `Tier 3 (400+ lbs)`: Max power knock-out sound effect

---

## DEMO
https://github.com/user-attachments/assets/6357e640-69b7-4168-8ffd-164a767968b1

