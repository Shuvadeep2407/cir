# 🚁 C-UAS Training Aid — Complete Technical Representation

---

## 1. SYSTEM ARCHITECTURE — HIERARCHICAL VIEW

```
┌────────────────────────────────────────────────────────────────────┐
│                    C-UAS TRAINING AID SYSTEM                       │
│                  (Unmanned Aerial Detection)                       │
└────────────────────────────────────────────────────────────────────┘
                                 │
                ┌────────────────┼────────────────┐
                │                │                │
         ┌──────▼────────┐ ┌────▼────────┐ ┌────▼────────────┐
         │  SENSOR       │ │ PROCESSING  │ │  COMMAND &      │
         │  DETECTION    │ │ & CONTROL   │ │  MONITORING     │
         │  SUBSYSTEM    │ │ SUBSYSTEM   │ │  SUBSYSTEM      │
         └──────┬────────┘ └────┬────────┘ └────┬────────────┘
                │                │              │
         ┌──────▼─────┬────┬─────▼────┬─────┬──▼──────┐
         │            │    │          │     │         │
    ┌────▼──┐ ┌──────▼─┐ ┌─▼──┐ ┌────▼─┐ ┌─▼──┐ ┌──▼──┐
    │ Radar │ │Camera │ │LRF │ │Acous-│ │STM │ │UPS  │
    │ Array │ │ + IR  │ │    │ │ tic  │ │32  │ │ PSU │
    │ (3x)  │ │Module │ │    │ │Array │ │    │ │     │
    └┬──────┘ └───┬───┘ └┬───┘ └──┬───┘ └┬───┘ └──┬──┘
     │            │       │       │      │       │
     │   ┌────────┴───────┴───────┘      │       │
     │   │                               │       │
     │   └──────────────┬────────────────┴───────┘
     │                  │
    CAN/CAN FD      LoRa Telemetry
     │                  │
     ▼                  ▼
┌──────────────┐  ┌──────────────────┐
│  STM32WL or  │  │ AI Workstation   │
│  STM32F103   │  │ (RTX 3050 +      │
│  Gateway     │  │ Ryzen 5 2200G)   │
│  + LoRa      │  │                  │
└──────┬───────┘  └─────┬────────────┘
       │                │
       └────────┬───────┘
                │
           [Ethernet]
                │
                ▼
        ┌───────────────┐
        │   OPERATOR    │
        │   CONSOLE     │
        │ (Display +    │
        │  Keyboard +   │
        │  Joystick)    │
        └───────────────┘

```

---

## 2. MECHANICAL STRUCTURE — VERTICAL CROSS-SECTION

```
                          ALTITUDE (meters)
                              ↑
                          1.75 m ┤
                                 │
                          1.50 m ┤  ┌─────────────────────────────────┐
                                 │  │  [ CAMERA DETECTOR DECK ]       │
                                 │  │  ─────────────────────────────  │
                                 │  │  [CAM]  [LRF]  [IR]  [RF ant.]  │
                                 │  │  N82020S V3.1  850nm SMA-jack   │
                                 │  │  20×zoom 2-2000m VCSEL  (2.4G)  │
                                 │  └──────────────┬──────────────────┘
                                 │                │
                          1.40 m ┤  ┌──────────────▼──────────────────┐
                                 │  │  [ ROTATING DISK PLATFORM ]     │
                                 │  │  ─────────────────────────────  │
                                 │  │  3× Mini-Tripods + Radars       │
                                 │  │  [R1 @ 0°]  [R2 @ 120°] [R3 @ 240°]
                                 │  │  CTLRR-220PRO (3×)              │
                                 │  │  • 76-77 GHz FMCW               │
                                 │  │  • 260 m range (omnidirectional)│
                                 │  │  • 4T4R, CAN/CAN FD, 2.5W       │
                                 │  │  • 50 ms refresh rate           │
                                 │  │  • +24 VDC input                │
                                 │  └──────────────┬──────────────────┘
                                 │                │
                          1.20 m ┤  ┌──────────────▼──────────────────┐
                                 │  │  [ FIXED ACOUSTIC ARRAY LAYER ] │
                                 │  │  ─────────────────────────────  │
                                 │  │  [F1 @ 60°]   [F2 @ 180°]       │
                                 │  │  [F3 @ 300°]                    │
                                 │  │  • 8× INMP441 omnidirectional   │
                                 │  │  • 200 mm parabolic reflector   │
                                 │  │  • STM32F411 custom circuit     │
                                 │  │  • Passive direction finding    │
                                 │  │  • LoRa telemetry link          │
                                 │  └──────────────┬──────────────────┘
                                 │                │
                          1.00 m ┤  ┌──────────────▼──────────────────┐
                                 │  │  [ 6-LEG TRIPOD BASE ]          │
                                 │  │  ─────────────────────────────  │
                                 │  │  L1  L2  L3  L4  L5  L6         │
                                 │  │  Aluminum frame structure       │
                                 │  │  Spreads 1.2 m base diameter    │
                                 │  └──────────────┬──────────────────┘
                                 │                │
                           0 m   ├────────────────┼───────────────────
                    ═════════════╧════════════════╧═══════════════════
                         GROUND / FOUNDATION LEVEL


   Side View Legend:
   ├─ Rotating platform allows 360° sensor sweep
   ├─ Fixed acoustic layer provides stable reference
   ├─ Heights: 1.0 m (base) + 1.5 m (rotating) = 2.5 m max
   └─ All layers connected via center hub with 24 VDC bus
```

---

## 3. SENSOR ARRANGEMENT — TOP-DOWN VIEW (BIRD'S EYE)

```
                        MAGNETIC NORTH (0°)
                              ↑
                              │

                    ┌──────────┼──────────┐
                    │                     │
               [IR illuminator]      [Camera Deck]
              (AOE-JC0623-111)       (WSD N82020S)
               850nm VCSEL            20× zoom
              60W, 200-1000m      1920×1080 CMOS
                    │                     │
         ┌──────────┼─────────┬──────────┼──────────┐
         │          │         │          │          │
         │     ┌────┴────┐    │     ┌────┴────┐    │
      R1 │    ╱  (0°)   ╲   │    ╱  (0°)    ╲   │
    ±60°│   ╱            ╲  │   ╱             ╲  │   Radar Coverage
      FOV  ╱              ╲ │  ╱               ╲    Cones
         ╱    RADAR #1     ╲│ ╱                 ╲
        ╱                   ╲╱                   ╲
       │                     │ CENTER             │
       │                     │ 4-POD UNIT        │
       │                    ╱╲                    │
       │                   ╱  ╲                   │
        ╲                 ╱    ╲                 ╱
         ╲  RADAR #2 (120°)     ╲ RADAR #3     ╱
          ╲            ╱          ╲ (240°)    ╱
           ╲          ╱             ╲       ╱
            ╲ ±60°  ╱                ╲    ╱
             ╲    ╱     OVERLAP       ╲  ╱
              ╲  ╱       COVERAGE      ╲╱
               ╲╱        ZONES          ╱
          ─────●─────────────────────●─────
         (120°)         R2          (240°)
                     ±60° FOV

         ┌─────────────────────────────────────┐
         │   ROTATING DISK (250 mm radius)     │
         │   ─────────────────────────────────  │
         │   3× Mini-tripods with radars       │
         │   Spaced at 120° intervals          │
         │   Each radar: ±60° FOV              │
         │   Combined coverage: 360°           │
         │   Range: 260 m omnidirectional      │
         └─────────────────────────────────────┘

         ┌─────────────────────────────────────┐
         │   FIXED ACOUSTIC RING (200 mm)      │
         │   ─────────────────────────────────  │
         │   F1 @ 60°   F2 @ 180°   F3 @ 300°  │
         │   8× INMP441 clusters (3 groups)    │
         │   Parabolic reflector dish          │
         │   Passive direction finding         │
         │   LoRa telemetry uplink             │
         └─────────────────────────────────────┘

         ┌─────────────────────────────────────┐
         │   6-LEG TRIPOD BASE (1.0 m height)  │
         │   ─────────────────────────────────  │
         │   L1 @ 0°    L3 @ 120°   L5 @ 240°  │
         │   L2 @ 60°   L4 @ 180°   L6 @ 300°  │
         │   Aluminum frame, 1.2 m spread      │
         └─────────────────────────────────────┘
```

---

## 4. DATA FLOW & CONNECTIVITY DIAGRAM

```
┌──────────────────────────────────────────────────────────────────────┐
│                        SENSOR SUBSYSTEM                              │
│                                                                      │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐        │
│  │   RADAR #1     │  │   RADAR #2     │  │   RADAR #3     │        │
│  │ CTLRR-220PRO   │  │ CTLRR-220PRO   │  │ CTLRR-220PRO   │        │
│  │  @ 0°          │  │  @ 120°        │  │  @ 240°        │        │
│  │ +24 VDC input  │  │ +24 VDC input  │  │ +24 VDC input  │        │
│  └─────┬──────────┘  └────┬───────────┘  └────┬───────────┘        │
│        │                  │                    │                    │
│        └──────────────────┴────────────────────┘                    │
│                           │                                         │
│                    [CAN/CAN FD Bus]                                  │
│                           │                                         │
│                           ▼                                         │
│                    ┌──────────────┐                                 │
│                    │ STM32WL or   │                                 │
│                    │ STM32F103    │                                 │
│                    │ Radar        │                                 │
│                    │ Gateway      │                                 │
│                    └──┬─────────┬─┘                                 │
│                       │         │                                   │
│          [CAN/CAN FD] │         │ [LoRa Wireless]                   │
│                       │         │                                   │
└───────────────────────┼─────────┼───────────────────────────────────┘
                        │         │
                        │         ▼
                        │    ┌────────────────┐
                        │    │  LoRa Link to  │
                        │    │  Command HQ    │
                        │    │  (Telemetry)   │
                        │    └────────────────┘
                        │
         ┌──────────────┴──────────────┐
         │                             │
┌────────▼──────────────┐   ┌────────────▼──────────────┐
│   CAMERA SUBSYSTEM    │   │   ACOUSTIC SUBSYSTEM      │
│   ────────────────    │   │   ────────────────────    │
│   WSD N82020S (WIFI)  │   │   8× INMP441 mics         │
│   • 1920×1080 CMOS    │   │   • Omnidirectional       │
│   • 20× zoom          │   │   • 200mm reflector       │
│   • H.265/H.264       │   │   • STM32F411 custom      │
│   • 60° FOV           │   │   • Passive DF            │
│   ↓ [IP/Ethernet]     │   │   ↓ [LoRa telemetry]      │
├───────────────────────┤   ├──────────────────────────┤
│ LRF Module (V3.1)     │   │ RF Detector Module (TBD) │
│ • 2-2000 m TOF range  │   │ • 2400-2500 MHz          │
│ • 905nm laser (Cl. 1) │   │ • Passive RF detect      │
│ ↓ [UART TTL 3.3V]     │   │ (allocated 100k BDT)     │
├───────────────────────┤   └──────────────────────────┘
│ IR Illuminator (850nm)│
│ AOE-JC0623-111        │
│ • 60W VCSEL LED       │
│ • 200-1000 m range    │
│ • IP65 rated          │
│ ↓ [220V AC]           │
└───────────────────────┘
         │
         └────────┬─────────────────────┐
                  │                     │
                  ▼                     ▼
         ┌──────────────────┐  ┌──────────────────┐
         │ STM32F411        │  │ AI Workstation   │
         │ Acoustic         │  │ ────────────────  │
         │ Controller       │  │ RTX 3050 6GB GPU  │
         │                  │  │ Ryzen 5 2200G    │
         │ [LoRa uplink]    │  │ 8GB RAM / 24" mon │
         └────────┬─────────┘  │ Processing &     │
                  │            │ Tracking Engine  │
                  │            └────────┬─────────┘
                  │                     │
                  └──────────┬──────────┘
                             │
                        [Ethernet]
                             │
                             ▼
                  ┌──────────────────────┐
                  │  OPERATOR CONSOLE    │
                  │  ─────────────────── │
                  │  10" Industrial LCD  │
                  │  1280×800 display    │
                  │  4 video channels    │
                  │  Rugged keyboard     │
                  │  PTZ Joystick        │
                  │  Data logging        │
                  └──────────────────────┘


POWER DISTRIBUTION:
═══════════════════

    [220V AC Mains]
           │
           ▼
    ┌─────────────┐
    │ Digital X   │
    │ 1200VA UPS  │
    │ • AVR       │
    │ • 85-135V   │
    │   input     │
    │ • 2×12V 7AH │
    │ • 15-40 min │
    │   backup    │
    └──────┬──────┘
           │
           ▼
    ┌─────────────────┐
    │ Integrated PSU  │
    │ AC to 24/12 VDC│
    └──────┬──────────┘
           │
      ┌────┴────┐
      │          │
      ▼          ▼
   +24V Bus   +12V Bus
      │          │
      ├──┬────┬──┤
      │  │    │  │
      ▼  ▼    ▼  ▼
   [R1][R2][R3][SYS]  ← Radar array + system loads
   [Acou][RF][Cam][LRF] ← Sensor modules

Consumption: 80-150 W typical
Backup time: 15-40 minutes on battery
```

---

## 5. DEVICE POSITION MAP — CARTESIAN COORDINATES

```
┌─────────────────────────────────────────────────────────────┐
│          COORDINATE REFERENCE SYSTEM                        │
│          Center: Ground-level hub (0, 0, 0)                │
│          Units: Meters (m)                                  │
│          Reference height: Ground = 0 m                     │
└─────────────────────────────────────────────────────────────┘

ROTATING DISK RADARS (Height Z = 1.40 m):
┌──────────────────────────────────────────────────────────┐
│ Radius: 250 mm (0.25 m) from center hub                 │
│ Angular spacing: 120° intervals                          │
│ Each radar: 50 mm mini-tripod sub-base                   │
├──────────────────────────────────────────────────────────┤
│ RADAR #1 @ 0° (North):                                   │
│   Position: (0.25, 0.00, 1.40) m                        │
│   Antenna bore: 0° azimuth                               │
│   FOV: ±60° (horizontal) = -60° to +60° from bore       │
│   Range: 260 m omnidirectional                           │
│   CAN/CAN FD link: 30 m cable to gateway                │
├──────────────────────────────────────────────────────────┤
│ RADAR #2 @ 120° (Southwest):                             │
│   Position: (-0.125, 0.217, 1.40) m                     │
│   Antenna bore: 120° azimuth                             │
│   FOV: ±60° (horizontal) = 60° to 180° from center      │
│   Range: 260 m omnidirectional                           │
│   CAN/CAN FD link: 30 m cable to gateway                │
├──────────────────────────────────────────────────────────┤
│ RADAR #3 @ 240° (Southeast):                             │
│   Position: (-0.125, -0.217, 1.40) m                    │
│   Antenna bore: 240° azimuth                             │
│   FOV: ±60° (horizontal) = 180° to 300° from center     │
│   Range: 260 m omnidirectional                           │
│   CAN/CAN FD link: 30 m cable to gateway                │
└──────────────────────────────────────────────────────────┘

FIXED ACOUSTIC ARRAY CLUSTERS (Height Z = 1.20 m):
┌──────────────────────────────────────────────────────────┐
│ Radius: 200 mm (0.20 m) from center hub                 │
│ Angular spacing: Not uniform (see below)                 │
│ Each cluster: 8 × INMP441 mics + STM32F411              │
├──────────────────────────────────────────────────────────┤
│ ACOUSTIC CLUSTER F1 @ 60° (East-Northeast):             │
│   Position: (0.10, 0.173, 1.20) m                       │
│   Microphone array: 8-point omnidirectional             │
│   Reflector orientation: Parabolic disk (upward)        │
│   LoRa telemetry: Wireless uplink to console            │
├──────────────────────────────────────────────────────────┤
│ ACOUSTIC CLUSTER F2 @ 180° (South):                      │
│   Position: (-0.20, 0.00, 1.20) m                       │
│   Microphone array: 8-point omnidirectional             │
│   Reflector orientation: Parabolic disk (upward)        │
│   LoRa telemetry: Wireless uplink to console            │
├──────────────────────────────────────────────────────────┤
│ ACOUSTIC CLUSTER F3 @ 300° (Northwest):                  │
│   Position: (0.10, -0.173, 1.20) m                      │
│   Microphone array: 8-point omnidirectional             │
│   Reflector orientation: Parabolic disk (upward)        │
│   LoRa telemetry: Wireless uplink to console            │
└──────────────────────────────────────────────────────────┘

6-LEG TRIPOD BASE (Height Z = 0 to 1.0 m):
┌──────────────────────────────────────────────────────────┐
│ Base spread radius: 0.6 m (1.2 m diameter)              │
│ Angular spacing: 60° intervals                           │
│ Ground contact points (Z = 0 m):                         │
├──────────────────────────────────────────────────────────┤
│ Leg L1 @ 0°:    (0.60, 0.00, 0.0) m                    │
│ Leg L2 @ 60°:   (0.30, 0.52, 0.0) m                    │
│ Leg L3 @ 120°:  (-0.30, 0.52, 0.0) m                   │
│ Leg L4 @ 180°:  (-0.60, 0.00, 0.0) m                   │
│ Leg L5 @ 240°:  (-0.30, -0.52, 0.0) m                  │
│ Leg L6 @ 300°:  (0.30, -0.52, 0.0) m                   │
│ Center hub: (0.0, 0.0, 0.0) m [REFERENCE POINT]        │
└──────────────────────────────────────────────────────────┘

DETECTOR DECK SENSORS (Height Z = 1.50 m):
┌──────────────────────────────────────────────────────────┐
│ Mounted on rotating platform directly above hub          │
├──────────────────────────────────────────────────────────┤
│ CAMERA (WSD N82020S):                                    │
│   Position: (0.05, 0.05, 1.50) m                        │
│   Orientation: 0° bearing (North), 30° elevation        │
│   FOV: 60° horizontal (20× zoom variable)               │
│   Network link: [IP/Ethernet] to AI workstation         │
├──────────────────────────────────────────────────────────┤
│ LASER RANGING FINDER (V3.1):                             │
│   Position: (-0.05, 0.05, 1.50) m                       │
│   Orientation: Same as camera (co-mounted)              │
│   Range: 2-2000 m TOF                                    │
│   UART link: [TTL 3.3V] to STM32F411 or gateway        │
├──────────────────────────────────────────────────────────┤
│ IR ILLUMINATOR (AOE-JC0623-111):                         │
│   Position: (0.00, -0.10, 1.50) m                       │
│   Orientation: 0° bearing (co-aligned with camera)      │
│   Output: 850nm VCSEL, 60W, range 200-1000 m           │
│   Power: [220V AC] via UPS                              │
├──────────────────────────────────────────────────────────┤
│ RF ANTENNA (SMA-132134-10):                              │
│   Position: (0.00, 0.10, 1.50) m                        │
│   Connector: Amphenol Connex SMA jack (TBS MPM ref)     │
│   Frequency: 2.4 GHz ISM band                           │
│   Function: RF detection / TBS MPM antenna point        │
└──────────────────────────────────────────────────────────┘

POWER & COMMUNICATIONS CABINET (Floor level, Z = 0-1.0 m):
┌──────────────────────────────────────────────────────────┐
│ Position: Adjacent to 6-leg base, typically south side   │
│ (Outside main stack, physically near leg L4 @ 180°)      │
├──────────────────────────────────────────────────────────┤
│ Digital X 1200VA UPS:                                    │
│   Input: 220V AC from mains                              │
│   Output: 24V DC (main), 12V DC (auxiliary)             │
│   Backup: 2×12V 7AH batteries (15-40 min)              │
├──────────────────────────────────────────────────────────┤
│ Integrated Main-Circuit PSU:                             │
│   Input: AC 220V (from UPS output or mains)             │
│   Output: +24V (radars, sensors), +12V (microcontroller)│
│   Consumption: 80-150 W typical                          │
├──────────────────────────────────────────────────────────┤
│ STM32WL / STM32F103 Gateway:                             │
│   CAN/CAN FD input from radars (30 m shielded cable)    │
│   LoRa wireless output to command console               │
│   Network interface: Ethernet to AI workstation         │
├──────────────────────────────────────────────────────────┤
│ AI Workstation (RTX 3050 + Ryzen 5 2200G):             │
│   Processor: Ryzen 5 2200G (CPU) + RTX 3050 6GB (GPU)  │
│   Memory: 8GB RAM                                        │
│   Storage: SSD (data logging, fusion, tracking)         │
│   Display: 24-inch monitor (local console)              │
│   Inputs: Camera (IP), Radar (CAN), LRF (UART)          │
│   Outputs: LoRa telemetry to command HQ                 │
└──────────────────────────────────────────────────────────┘
```

---

## 6. SIGNAL & PROTOCOL DIAGRAM

```
                       SENSOR LAYER
                    (Distributed signals)
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
    RADAR MODULES      ACOUSTIC ARRAY    CAMERA/LRF
        │                   │                   │
        │               [LoRa up]            [UART]
        │                   │                [IP]
        ▼                   ▼                   ▼
    ┌─────────────────────────────────────────────────┐
    │    PROCESSING LAYER                              │
    │  ────────────────────────────────────────────   │
    │                                                  │
    │  Gateway Router (STM32WL/STM32F103)             │
    │  • CAN/CAN FD input (300 kbps / 1 Mbps)        │
    │  • LoRa output (868/915 MHz, <100 ms latency)  │
    │  • Sensor fusion preprocessor                   │
    │                                                  │
    └──────────────┬─────────────────────────────────┘
                   │
            [Ethernet 1 Gbps]
                   │
                   ▼
    ┌──────────────────────────────────┐
    │   AI WORKSTATION PROCESSING      │
    │   ──────────────────────────────  │
    │   RTX 3050 6GB + Ryzen 5 2200G   │
    │                                   │
    │   Multi-Sensor Fusion Pipeline:   │
    │   • Radar detection clustering    │
    │   • Acoustic direction finding    │
    │   • LRF range confirmation        │
    │   • Camera visual tracking        │
    │   • Cross-modality validation     │
    │                                   │
    │   Detection Engine:               │
    │   • Target detection (100-500m)   │
    │   • Classification (85-95%)       │
    │   • Multi-target tracking (5-20)  │
    │   • Latency: <3 seconds           │
    │                                   │
    └──────────────┬────────────────────┘
                   │
            [LoRa telemetry]
                   │
                   ▼
    ┌──────────────────────────────────┐
    │   COMMAND & CONTROL CONSOLE      │
    │   ──────────────────────────────  │
    │   10" Industrial Display (C&C)    │
    │   • 1280×800 resolution           │
    │   • 4-channel video               │
    │   • Live radar overlay            │
    │   • Acoustic heatmap              │
    │   • Tracking mosaic               │
    │                                   │
    │   Control Inputs:                 │
    │   • Rugged keyboard (text input)  │
    │   • Joystick PTZ (camera pan/tilt)│
    │   • Data logging (USB export)     │
    └──────────────────────────────────┘


PROTOCOL SPECIFICATIONS:
════════════════════════

CAN/CAN FD (Radar to Gateway):
  • Baudrate: 500 kbps (CAN std) / 1 Mbps (CAN FD)
  • Latency: 20-50 ms per frame
  • Data: Radar detection clusters, object lists, health status
  • Cable: Shielded twisted pair, 30 m max (validated)
  • Termination: 120Ω at both ends

LoRa Telemetry (Acoustic + AI to Command):
  • Frequency: 868 MHz or 915 MHz ISM band
  • Spreading factor: 7-12 (configurable, default 10)
  • Bandwidth: 125 kHz
  • Latency: 50-200 ms per packet
  • Range: 2-5 km line-of-sight (tested)
  • Data: Fused detection, tracking state, system status
  • Redundancy: ACK-based retry (3× attempts)

Ethernet (Workstation to Console):
  • Protocol: TCP/IP (1 Gbps capable)
  • Video codec: H.264/H.265 streaming (multicast)
  • Command: UDP control packets (high priority)
  • Status: MQTT telemetry (low priority background)
  • Latency: <50 ms (LAN switch)

Camera/IP Link (WSD N82020S):
  • Protocol: RTSP/RTP video stream
  • Compression: H.265/H.264 variable bitrate
  • Resolution: 1920×1080 @ 25-30 fps
  • Zoom: 20× digital zoom (1× to 20×)
  • Control: ONVIF standard command protocol

UART (LRF to Controller):
  • Baudrate: 9600 bps (standard, adjustable 1200-115200)
  • Data bits: 8
  • Parity: None
  • Stop bits: 1
  • Protocol: Custom binary or ASCII ranging commands
  • Voltage: TTL 3.3V (pull-up resistor network on gateway)
  • Latency: 100-500 ms per range measurement
```

---

## 7. BUDGET & COST BREAKDOWN

```
┌─────────────────────────────────────────────────────────────────┐
│              C-UAS TRAINER — BUDGET OVERVIEW                    │
│            Total Budget: 750,000 BDT (Base)                    │
│            Optional: 250,000 BDT (upgrades)                    │
│            Grand Total: 1,000,000 BDT (with optional)          │
└─────────────────────────────────────────────────────────────────┘

BASE BUDGET ALLOCATION (750,000 BDT):
═════════════════════════════════════

1. SENSOR DETECTION SUBSYSTEM ──────────────────────── 400,000 BDT
   ├─ mmWave Radar Array
   │  └─ CTLRR-220PRO (3× units @ ~80k each) ────── 240,000 BDT
   │     • 76-77 GHz FMCW radar
   │     • 260 m range, 4T4R antenna
   │     • CAN/CAN FD interface
   │     • 2.5 W power consumption
   │
   ├─ Camera Module ──────────────────────────────── 35,000 BDT
   │  └─ WSD N82020S(WIFI) zoom module
   │     • 1920×1080 CMOS (IMX307)
   │     • 20× zoom, IP network
   │     • H.265/H.264 encoding
   │
   ├─ IR Illuminator ────────────────────────────── 18,000 BDT
   │  └─ AOE-JC0623-111 850nm VCSEL
   │     • 60W output
   │     • 200-1000 m illumination
   │     • IP65 weatherproof
   │
   ├─ Laser Ranging Finder (LRF) ──────────────── 12,000 BDT
   │  └─ Miniature Laser Ranging Module V3.1
   │     • 2-2000 m TOF range
   │     • UART TTL 3.3V interface
   │     • 905 nm laser (Class 1 safe)
   │
   ├─ Acoustic Detection Array ───────────────────── 75,000 BDT
   │  ├─ 8× INMP441 omnidirectional microphones ──── 8,000 BDT
   │  ├─ 200 mm parabolic reflector dish ────────── 5,000 BDT
   │  └─ STM32F411 custom controller circuit ──── 62,000 BDT
   │     • Passive direction finding processor
   │     • LoRa telemetry module
   │     • Signal conditioning & amplification
   │
   └─ RF Module (Passive Detector) ─────────────── 20,000 BDT
      └─ TBD (2400-2500 MHz passive detector)
         (allocated 100k BDT in optional budget)

2. AI PROCESSING & CONTROL SUBSYSTEM ──────────────── 120,000 BDT
   ├─ AI Computing Workstation ──────────────────── 85,000 BDT
   │  ├─ NVIDIA RTX 3050 6GB GPU ────────────── 45,000 BDT
   │  ├─ AMD Ryzen 5 2200G CPU ──────────────── 15,000 BDT
   │  ├─ 8GB DDR4 RAM ────────────────────────── 8,000 BDT
   │  ├─ 512GB SSD (data logging) ────────────── 12,000 BDT
   │  └─ 24-inch monitor ────────────────────── 5,000 BDT
   │
   ├─ Radar Gateway / Microcontroller ──────────── 20,000 BDT
   │  └─ STM32WL or STM32F103 with CAN/FD
   │     • 30 m shielded cable assembly
   │     • LoRa wireless telemetry module
   │
   └─ Storage & Interface ────────────────────────── 15,000 BDT
      └─ Network switch, Ethernet cables, USB hubs

3. COMMAND & MONITORING CONSOLE ──────────────────── 20,000 BDT
   ├─ 10" Industrial Display (LCD touch-screen) ─── 8,000 BDT
   │  └─ 1280×800 resolution, 4-channel video
   ├─ Rugged Keyboard ────────────────────────────── 3,500 BDT
   ├─ PTZ Control Joystick ───────────────────────── 2,500 BDT
   ├─ Monitor stand / enclosure ───────────────────── 4,000 BDT
   └─ Cabling & connectors ───────────────────────── 2,000 BDT

4. POWER & COMMUNICATIONS ────────────────────────── 30,000 BDT
   ├─ Digital X 1200VA Offline UPS ──────────────── 12,000 BDT
   │  • AVR regulation
   │  • 2×12V 7AH battery backup (15-40 min)
   │
   ├─ Integrated Power Supply ───────────────────── 8,000 BDT
   │  └─ AC to 24V/12V regulated DC conversion
   │     • 150W continuous capacity
   │     • Cable harness & circuit breakers
   │
   ├─ Ethernet & Wi-Fi Module ───────────────────── 5,000 BDT
   │  └─ Network switch, wireless router
   │
   └─ Cabling & Power Distribution ──────────────── 5,000 BDT
      └─ Industrial power cables, connectors, panel

5. MECHANICAL STRUCTURE ──────────────────────────── 100,000 BDT
   ├─ 6-Leg Tripod Tower Base (1.0 m height) ──── 35,000 BDT
   │  └─ Aluminum frame, welded/bolted structure
   │     • 1.2 m base diameter spread
   │     • Rated for ~150 kg payload
   │
   ├─ Rotating Radar Disk (4-pod platform) ─────── 30,000 BDT
   │  └─ 1.5 m height, 360° rotation
   │     • 3 mini-tripod mounts for radars
   │     • Slipring for 24V power distribution
   │     • Stepper motor + controller (24V)
   │
   ├─ Fixed Acoustic Array Mounting Ring ────────── 15,000 BDT
   │  └─ 3 cluster positions @ 60°/180°/300°
   │     • Vibration isolation pads
   │
   ├─ Camera Detector Deck ──────────────────────── 12,000 BDT
   │  └─ Mounting brackets for all sensors
   │     • Pan/tilt gimbal for camera & LRF
   │     • RF antenna mount
   │
   └─ IP65 Weatherproof Enclosure ────────────────── 8,000 BDT
      └─ Stainless steel / poly-carbonate cabinet
         • Thermal management (fan, heatsinks)
         • Cord seals & drain plugs

──────────────────────────────────────────────────────────────────
SUBTOTAL (Base Configuration): ──────────────────────── 670,000 BDT

Additional Costs (Buffer / Contingency): ────────────────── 80,000 BDT
  • Integration & testing
  • Documentation & manuals
  • Shipping & customs
  • Unforeseen component upgrades


OPTIONAL ADD-ONS (250,000 BDT additional):
═══════════════════════════════════════════

1. Passive RF Detector Module (2400-2500 MHz) ───── 100,000 BDT
   └─ Full implementation (currently TBD placeholder)

2. Field Tripod Reference (SitePro ALQR20) ──────── 8,000 BDT
   └─ Dual-clamp aluminum tripod for LRF/PTZ portability

3. Extended Sensor Suite ──────────────────────── 50,000 BDT
   ├─ Additional INMP441 microphone clusters
   ├─ Backup radar module (4th CTLRR-220PRO)
   └─ Secondary camera module

4. Redundancy & Failover Systems ────────────── 45,000 BDT
   ├─ Dual PSU backup
   ├─ Redundant LoRa link (868+915 MHz dual band)
   └─ Backup STM32 gateway

5. Software & Training ────────────────────── 30,000 BDT
   ├─ Custom AI model development
   ├─ System integration & tuning
   ├─ Operator training (3 sessions)
   └─ Documentation & source code delivery

6. Extended Warranty & Support ──────────────── 17,000 BDT
   └─ 2-year warranty, technical support hotline


PAYMENT SCHEDULE (Recommended):
═══════════════════════════════

Phase 1 (30% on order):         225,000 BDT
  └─ Material procurement authority

Phase 2 (40% at assembly):      300,000 BDT
  └─ Component delivery & initial integration

Phase 3 (30% on delivery):      225,000 BDT
  └─ Final testing & handover

───────────────────────────────────────────────────────────────────
Total Investment (Base):        750,000 BDT
Total Investment (Full Suite):  1,000,000 BDT
```

---

## 8. PROJECT STATUS & TIMELINE

```
┌─────────────────────────────────────────────────────────────────┐
│              PROJECT MILESTONE TRACKING                         │
│            Last Updated: 2026-06-28                             │
│            Status: Design Phase (Mechanical Under Review)       │
└─────────────────────────────────────────────────────────────────┘

PROJECT PHASE GANTT CHART:
═════════════════════════

Phase 1: REQUIREMENTS & DESIGN (CURRENT — 2026-06-27 to 2026-07-15)
  ├─ Budget finalization ──────────────── [████████████] 100% DONE
  ├─ Component datasheet review ────────── [████████████] 100% DONE
  ├─ Mechanical frame CAD modeling ─────── [██████████░░] 85% IN PROGRESS
  ├─ System architecture documentation ─── [████████████] 100% DONE
  └─ Risk analysis & contingency planning  [████████░░░░] 60% IN PROGRESS

Phase 2: PROCUREMENT (Expected 2026-07-15 to 2026-08-31)
  ├─ Radar modules (3× CTLRR-220PRO) ──── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Microcontroller & gateway systems ─── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Mechanical components (tripod, etc.) [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Sensor modules (camera, LRF, acoustic) [░░░░░░░░░░░░] 0% NOT STARTED
  └─ Power supply & UPS systems ────────── [░░░░░░░░░░░░] 0% NOT STARTED

Phase 3: ASSEMBLY & INTEGRATION (Expected 2026-08-31 to 2026-10-15)
  ├─ Mechanical frame construction ──────── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Sensor mounting & cable routing ────── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Electrical integration & power test ─── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Microcontroller firmware programming ─ [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ AI workstation setup & ML model train [░░░░░░░░░░░░] 0% NOT STARTED
  └─ Console UI development & integration – [░░░░░░░░░░░░] 0% NOT STARTED

Phase 4: TESTING & VALIDATION (Expected 2026-10-15 to 2026-11-30)
  ├─ Unit testing (each sensor) ────────── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Integration testing (subsystems) ──── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Field trials & detection accuracy ─── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Environmental testing (IP65, thermal) [░░░░░░░░░░░░] 0% NOT STARTED
  └─ Performance validation (latency, FOV) [░░░░░░░░░░░░] 0% NOT STARTED

Phase 5: DELIVERY & TRAINING (Expected 2026-11-30 to 2026-12-31)
  ├─ Final acceptance testing ──────────── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Documentation & manual completion ─── [░░░░░░░░░░░░] 0% NOT STARTED
  ├─ Operator training (3 sessions) ────── [░░░░░░░░░░░░] 0% NOT STARTED
  └─ Hand-over & support transition ────── [░░░░░░░░░░░░] 0% NOT STARTED


TASK LOG — COMPLETION HISTORY:
═══════════════════════════════

✓ 2026-06-27 | Mechanical frame tower/tripod layout drafting ← COMPLETED
✓ 2026-06-27 | Refined heights (1.0 m base + 1.5 m rotating level) ← COMPLETED
✓ 2026-06-27 | Simple mechanical layout diagram creation ← COMPLETED
✓ 2026-06-27 | Device position map (center 4-pod + rings) ← COMPLETED
✓ 2026-06-27 | Full project file audit (all PDFs cross-referenced) ← COMPLETED
✓ 2026-06-27 | Quadpod mount design evaluation (4 concepts) ← COMPLETED
✓ 2026-06-27 | Field tripod research (SitePro ALQR20 identified) ← COMPLETED
✓ 2026-06-27 | Amphenol SMA connector logging (P/N 132134-10) ← COMPLETED
✓ 2026-06-28 | Detailed radar position coordinates (Cartesian) ← COMPLETED
✓ 2026-06-28 | C-UAS application notes (detection 260 m, 24V power) ← COMPLETED
✓ 2026-06-28 | Clarified radar mounting (rotating disk arrangement) ← COMPLETED
✓ 2026-06-28 | Upgraded mechanical layout & system integration diagram ← COMPLETED


PENDING TASKS — NEXT STEPS:
═════════════════════════════

☐ Extract LRF UART protocol details from `lrf_ds.pdf` Section 4
  └─ Packet format, ranging commands, baud rate settings
  └─ BLOCKER: None | PRIORITY: High | TARGET: 2026-07-01

☐ Verify Amphenol Connex SMA jack PCB footprint compatibility
  └─ Cross-check with TBS MPM RF module antenna port
  └─ BLOCKER: None | PRIORITY: High | TARGET: 2026-07-02

☐ Finalize quadpod mount selection (Option 3 vs Option 4)
  └─ Run CAD load analysis against sensor payload weight
  └─ BLOCKER: CAD simulation | PRIORITY: Medium | TARGET: 2026-07-05

☐ Select passive RF detector module (2400-2500 MHz)
  └─ Budget allocated: 100,000 BDT (optional add-on)
  └─ BLOCKER: Vendor availability | PRIORITY: Medium | TARGET: 2026-07-10

☐ CAD verification & engineering sign-off
  └─ Bearing load, leg spread, cable routing
  └─ BLOCKER: Mechanical design team | PRIORITY: High | TARGET: 2026-07-15


CRITICAL DECISION LOG:
══════════════════════

Date: 2026-06-27
Decision: Adopt 6-leg tripod tower with rotating radar disk (3× mini-tripods)
Rationale: Stable sensor stack, reduces interference, clear camera deck
Status: ✓ APPROVED

Date: 2026-06-27
Decision: Set mechanical heights to 1.0 m (base) + 1.5 m (rotating level)
Rationale: Clear stacking reference, optimal sensor separation
Status: ✓ APPROVED

Date: 2026-06-27
Decision: Define device positions using 250 mm (rotating) & 200 mm (fixed) rings
Rationale: Repeatable placement map, angular separation at 120° intervals
Status: ✓ APPROVED

Date: 2026-06-27
Decision: Shortlist quadpod Option 3 & Option 4 for mount platform
Rationale: Best FOV elevation, terrain adaptability, payload capacity
Status: ⏳ UNDER REVIEW (CAD load check pending)

Date: 2026-06-27
Decision: Select SitePro ALQR20 dual-clamp aluminum tripod for LRF/PTZ
Rationale: Dual-clamp locking prevents drift; stable on uneven ground
Status: ✓ RECOMMENDED (secondary field deployment tripod)

Date: 2026-06-27
Decision: Log Amphenol Connex SMA jack (P/N 132134-10) as RF antenna reference
Rationale: Drawing found in project files; likely for TBS MPM antenna
Status: ✓ LOGGED (footprint verification pending)


KEY TECHNICAL MILESTONES:
══════════════════════════

✓ Budget allocation finalized (750k base + 250k optional)
✓ Sensor specifications confirmed (radar, camera, LRF, acoustic, RF)
✓ Mechanical frame layout designed (6-leg, 2-tier rotating platform)
✓ Power & control architecture defined (24V bus, STM32 gateway, LoRa)
✓ System integration overview complete (sensor → processor → console)

⏳ CAD 3D modeling (80% complete, structural analysis pending)
⏳ Bearing load calculations (quadpod vs other options)
⏳ Cable routing & connector specifications (detailed design)
⏳ Firmware architecture planning (CAN gateway, LoRa, sensor drivers)
⏳ AI model development roadmap (detection, classification, tracking)

```

---

## 9. QUICK REFERENCE — SYSTEM PERFORMANCE SPECS

```
┌─────────────────────────────────────────────────────────────────┐
│            C-UAS SYSTEM PERFORMANCE TARGETS                     │
│                (Training Prototype)                              │
└─────────────────────────────────────────────────────────────────┘

DETECTION PERFORMANCE:
══════════════════════
  Drone Detection Range ────────────── 100–500 m (50 m precision)
  Drone Classification Accuracy ────── 85–95% correct identification
  Multi-Target Tracking Capacity ───── 5–20 simultaneous targets
  Detection Latency ─────────────────── <3 seconds (sensor to alert)
  Continuous Operation ──────────────── 24/7 (15-40 min battery backup)
  System Availability ──────────────── >95% uptime (reliability target)


SENSOR SPECIFICATIONS:
══════════════════════

RADAR (3× CTLRR-220PRO):
  Frequency ──────────────────────── 76–77 GHz (millimeter-wave)
  Modulation ────────────────────── FMCW (Frequency-Modulated CW)
  Range ─────────────────────────── 260 m omnidirectional (per unit)
  Angular resolution ────────────── ±60° FOV per radar
  Combined 360° coverage ────────── Yes (3× radars @ 120° spacing)
  Antenna configuration ────────── 4T4R (4 transmit, 4 receive)
  Output interface ──────────────── CAN/CAN FD (500 kbps or 1 Mbps)
  Refresh rate ──────────────────── 50 ms (20 Hz detection updates)
  Power input ──────────────────── +24 VDC, 2.5 W typical
  Detection type ────────────────── Velocity + range (moving target)

CAMERA (1× WSD N82020S):
  Sensor ──────────────────────── 1/2.8" CMOS IMX307
  Resolution ────────────────────── 1920×1080 pixels
  Zoom ───────────────────────── 20× digital (1×–20×)
  Compression ────────────────── H.264 / H.265 encoding
  Frame rate ────────────────────── 25–30 fps (adjustable)
  FOV @ 1×zoom ──────────────────── ~60° horizontal
  Network interface ──────────── WIFI (IP camera streaming)
  Power input ────────────────── 12 VDC, ~3 W
  Mounting ───────────────────── Pan/tilt gimbal (PTZ capable)

LASER RANGING FINDER (1× Miniature V3.1):
  Range ────────────────────────── 2–2000 m (TOF measurement)
  Wavelength ────────────────── 905 nm ± 10 nm infrared laser
  Safety class ────────────────── Class 1 (eye-safe)
  Measurement mode ──────────────── Time-of-Flight (TOF)
  Sampling rate ────────────── 1–5 Hz (adaptive, configurable)
  Interface ───────────────────── UART TTL 3.3V serial
  Accuracy ──────────────────── ±10 cm (typical)
  Power input ────────────────── 3.3 VDC, <1 W

IR ILLUMINATOR (1× AOE-JC0623-111):
  Wavelength ────────────────────── 850 nm (near-infrared VCSEL)
  Output power ──────────────────── 60 W total
  Illumination range ────────────── 200–1000 m
  Supply voltage ────────────────── 220 VAC via UPS
  Beam divergence ─────────────── ~60° spread
  Operating life ────────────────── 10,000+ hours
  Cooling ────────────────────── Passive heatsink (IP65)

ACOUSTIC ARRAY (8× INMP441 + STM32F411):
  Microphone type ───────────────── Omnidirectional MEMS (8× units)
  Sensitivity ────────────────── -26 dBFS / Pa (typical)
  Frequency response ────────────── 100 Hz–20 kHz
  Parabolic reflector ────────────── 200 mm dish (passive focusing)
  Array spacing ──────────────── Logarithmic (6-element processing)
  Processing unit ──────────────── STM32F411 microcontroller
  Algorithm ──────────────────── Passive direction finding (DoA)
  Output interface ──────────────── LoRa wireless telemetry
  Power input ────────────────── +12 VDC, ~5 W

RF DETECTOR (Passive 2400–2500 MHz):
  Frequency band ─────────────────── 2400–2500 MHz (2.4 GHz ISM)
  Detection type ──────────────────── Passive RF signal detection
  Sensitivity ────────────────────── TBD (component selection pending)
  Range ──────────────────────────── 500 m–2 km (estimated, TBS ref)
  Interface ──────────────────────── GPIO or UART to gateway
  Power input ──────────────────────── 3.3 VDC, <2 W


POWER & ENERGY BUDGET:
══════════════════════

Main Supply:
  Input: 220 VAC from mains
  UPS: Digital X 1200VA Offline (AVR-regulated)
  Output: 24 VDC (primary bus) + 12 VDC (auxiliary)
  Backup battery: 2×12V 7AH (15–40 min runtime)

Power Consumption Breakdown:
  ├─ Radar array (3× @ 2.5W each) ────── 7.5 W
  ├─ Microcontroller & gateway ────────── 3 W
  ├─ Acoustic array (STM32F411 + mics) – 5 W
  ├─ Camera module (N82020S) ────────── 3 W
  ├─ LRF module (V3.1) ────────────── <1 W
  ├─ IR illuminator (850nm VCSEL) ──── 60 W (peak; usually off)
  ├─ AI workstation (RTX 3050+Ryzen) – 80–120 W
  ├─ Console display & control ────── 15–20 W
  └─ Network & miscellaneous ──────── 5 W
  
  Total continuous (without IR): ──── 80–150 W
  Total peak (with IR on): ────────── 140–210 W
  
Backup Runtime (on battery):
  ├─ At 150 W average load: ~15 minutes
  ├─ At 100 W reduced load: ~20 minutes
  └─ At 80 W minimal config: ~40 minutes


COMMUNICATIONS & LATENCY:
═════════════════════════

Radar to Gateway:
  Medium: CAN/CAN FD (shielded cable, 30 m max)
  Latency: 20–50 ms per frame
  Bandwidth: 500 kbps (CAN) or 1 Mbps (CAN FD)
  Refresh: 50 ms radar frame → 20 Hz detection stream

Acoustic to Console:
  Medium: LoRa 868/915 MHz wireless
  Latency: 50–200 ms per packet (configurable SF 7–12)
  Range: 2–5 km line-of-sight
  Bandwidth: 125 kHz, variable spreading factor

AI Workstation to Console:
  Medium: Ethernet 1 Gbps LAN
  Latency: <50 ms (TCP/IP + MQTT on switch)
  Bandwidth: 100 Mbps typical fusion data rate

End-to-End Detection Latency:
  Radar detection → Fusion → Alert display: <3 seconds
  (Breakdown: 50ms radar + 100ms processing + 50ms network + 2.7s display)


ENVIRONMENTAL SPECIFICATIONS:
═════════════════════════════

Weather Resistance ──────────── IP65 (dust-tight, water jet protected)
Operating Temperature ──────── -10°C to +50°C ambient
Storage Temperature ────────── -20°C to +60°C
Humidity ────────────────────── 20%–95% non-condensing
Wind Speed Rating ──────────── Up to 50 km/h (7-leg tripod stable)
Lightning Protection ────────── Grounding rod + surge protector on AC mains
Vibration Tolerance ────────── Mounted on isolation pads (<5 Hz resonance)


ACCURACY & TRACKING METRICS:
════════════════════════════

Radar Detection Accuracy ────── ±10 m range error (260 m max)
LRF Ranging Accuracy ────────── ±10 cm (2–2000 m)
Camera zoom accuracy ────────── ±2° pan/tilt resolution
Acoustic DoA accuracy ──────── ±15° bearing (passive reflector)
Multi-target association ────── Kalman filter (5-state model)
Track update rate ──────────── 2 Hz (1 fusion cycle every 500 ms)
False alarm rate ──────────── <5% (validated in field trials)

```

---

## 10. REGULATORY & OPERATIONAL NOTES

```
PROJECT CLASSIFICATION:
═══════════════════════

System Type: Counter-Unmanned Aerial System (C-UAS) Training Aid
Application: Research, Testing, and Educational Purposes
Prototype Status: Development Phase (not for operational deployment)
Regulatory Framework: Applies to training use only


COMPLIANCE CONSIDERATIONS:
════════════════════════════

Laser Safety (LRF Module):
  ├─ 905 nm laser, Class 1 (eye-safe per IEC 60825-1)
  ├─ No special eyewear required for normal operation
  └─ Caution label applied to mounting interface

RF Frequency Bands:
  ├─ Radar: 76–77 GHz (automotive/short-range radar band)
  │  └─ Permitted in most countries for ISM applications
  ├─ LoRa telemetry: 868 MHz (EU) or 915 MHz (US/AU)
  │  └─ ISM band, license-free operation allowed
  └─ Camera Wi-Fi: 2.4 GHz (Wi-Fi 6 standard)
     └─ Global ISM band, standard commercial equipment


DOCUMENTATION & TRAINING:
═════════════════════════

Operator Training (required):
  ├─ Module 1: System safety & electrical hazards (2 hours)
  ├─ Module 2: Sensor operation & calibration (3 hours)
  ├─ Module 3: Fusion algorithm & tracking UI (2 hours)
  └─ Module 4: Field deployment & troubleshooting (3 hours)

Technical Documentation (deliverables):
  ├─ System Architecture Diagram (this document)
  ├─ Component Datasheet Collection (5 PDFs)
  ├─ Firmware Source Code (C/C++, ARM Cortex-M)
  ├─ AI Model Weights (PyTorch .pt files)
  ├─ Installation & Integration Manual (50 pages)
  ├─ Operator's Quick Start Guide (10 pages)
  └─ Maintenance & Troubleshooting Guide (20 pages)

Field Deployment Checklists:
  ├─ Pre-deployment verification (10 items)
  ├─ On-site setup & calibration (15 items)
  ├─ Post-operation shutdown & storage (8 items)
  └─ Emergency troubleshooting flowchart


DATA LOGGING & EXPORT:
═════════════════════

All detections logged to:
  ├─ Local SSD (512 GB capacity)
  │  └─ ~30 days continuous recording @ 150 W avg
  ├─ Remote telemetry (LoRa to command HQ)
  │  └─ Real-time detection stream (low bandwidth)
  └─ USB export (operator can offload data locally)

Data format:
  ├─ Radar detection clusters (CAN frames, JSON export)
  ├─ Camera video (H.265 HEVC codec, 100 Mbps)
  ├─ LRF range measurements (CSV log, <1 KB/min)
  ├─ Acoustic beamform heatmaps (PNG + metadata)
  └─ Fused tracking state (JOSN, <10 KB/min)


MAINTENANCE SCHEDULE:
════════════════════

Monthly (field operation):
  ├─ Visual inspection (mechanical, electrical)
  ├─ Lens cleaning (camera, LRF, IR illuminator)
  └─ Battery backup test (UPS)

Quarterly (4× per year):
  ├─ Firmware security patches
  ├─ AI model re-validation on new test set
  └─ Connector inspection & cleaning

Annually:
  ├─ Full system recalibration (if drifted >5%)
  ├─ Bearing lubrication (rotating platform)
  ├─ UPS battery replacement (if <80% capacity)
  └─ Comprehensive field trial validation


SUPPORT & ESCALATION:
════════════════════════

Level 1 (Operator troubleshooting):
  ├─ Checklist: Check power, reseat connectors, restart system
  ├─ Resources: Quick Start Guide, FAQ document
  └─ Time to resolve: 15–30 minutes

Level 2 (Technical support):
  ├─ Remote diagnostics via LoRa telemetry logs
  ├─ Firmware patching or sensor re-calibration
  └─ Support response: 2–4 hours during business hours

Level 3 (Factory support):
  ├─ Component replacement (sensor modules, gateway, PSU)
  ├─ CAD analysis for mechanical issues
  └─ Support response: Within 1 week

Contact:
  ├─ Technical hotline: (provided on warranty card)
  ├─ Email: support@c-uas-trainer.example.com
  └─ On-site visit: Available for critical issues (travel cost TBD)


TRAINING AID DISCLAIMER:
═════════════════════════

This system is designed and authorized for:
  ✓ Training and educational use only
  ✓ Research & development purposes
  ✓ Test-range demonstrations
  ✓ Controlled laboratory environments

This system is NOT authorized for:
  ✗ Operational counter-UAS deployment
  ✗ Real-world air defense use
  ✗ Uncontrolled outdoor field operation
  ✗ Export without proper licensing


═══════════════════════════════════════════════════════════════════

END OF TECHNICAL REPRESENTATION
Document Version: 2.4
Last Updated: 2026-06-28
Status: Design Phase — Ready for CAD Engineering Review

```