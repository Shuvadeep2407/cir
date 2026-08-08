# 🚁 Distributed C-UAS Training Aid — Complete Technical Representation

---

## 1. SYSTEM ARCHITECTURE — DISTRIBUTED CONSTELLATION VIEW

The system utilizes a **Distributed Field Constellation Topology** spread across an $800\text{ m}$ tactical footprint rather than a single vertically stacked tower. Remote satellite sensor nodes process information locally and utilize long-range wireless telemetry to communicate back to the master central command station.

```
                                 [0° NORTH]
                                     │
                        Acoustic Node #1 (R = 280m)
                                     │
               (300°)                │                (60°)
         Radar Node #3 ▀▄            │            ▄▀ Radar Node #1
          (R = 250m)     ▀▄          │          ▄▀     (R = 250m)
                           ▀▄        │        ▄▀
                             ▀▄   ┌──┴──┐   ▄▀
                               ▀▄ │     │ ▄▀
                                 ─┤Master├─
       ───────────────────────────┤  EO  ├─────────────────────────── [EAST]
       [WEST]                     │Station│                      (90°)
                                 ─┤(0,0,0)├─
                               ▀▄ │     │ ▄▀
                                 ▄▀└──┬──┘▄▀
                               ▄▀     │    ▀▄
                             ▄▀       │      ▀▄
               Radar Node #2▄▀        │        ▀▄Acoustic Node #2
                (R = 250m)            │           (R = 280m)
               (180°)                 │                (120°)
                                     │
                        Acoustic Node #3 (R = 280m)
                                   (240°)

```

---

## 2. CONSTELLATION NODE BREAKDOWN

### Master Central EO Command Station (Coordinate Origin: 0, 0, 0)

The heart of the system is the localized command center placed on a rugged field tripod base.

* **EO Sensor Pod (4-Pod Array):** Houses co-aligned visual and electro-optical identification modules.


* **Camera Module:** WSD N82020S featuring a 1/2.8" CMOS IMX307 sensor with 1920×1080 resolution and a 20× variable digital zoom.


* **Laser Range Finder (LRF):** Miniature Laser Ranging Module V3.1 operating an eye-safe 905nm Class 1 laser with a Time-of-Flight (TOF) range of 2–2000 meters.


* **IR Illuminator:** AOE-JC0623-111 60W near-infrared VCSEL LED offering a 200–1000 meter illumination beam.


* **RF Antenna:** SMA-132134-10 Amphenol Connex SMA jack antenna point for passive RF signal interception across the 2.4 GHz ISM band.




* **Processing & Control Enclosure:** Sits at the tripod base to consolidate field data.


* **AI Workstation:** High-performance processing core containing an NVIDIA RTX 3050 6GB GPU and an AMD Ryzen 5 2200G CPU paired with 8GB RAM and a 512GB logging SSD.


* **Operator Console:** Features a 10" industrial 1280×800 LCD display supporting a 4-channel live video feed with integrated radar tracking overlays, a rugged keyboard, and a dedicated PTZ control joystick.


* **Power Base:** Contains an integrated AC to 24V/12V DC power supply unit backed up by a Digital X 1200VA Offline UPS system equipped with internal 2×12V 7AH batteries for 15–40 minutes of continuous runtime.





### Distributed Radar Outposts (3x Units Configured at 250m Radius)

Three independent, star-bracketed outriggers form an overlapping $360^{\circ}$ perimeter scan loop.

* **Active Sensor Array:** Each outpost mounts a CTLRR-220PRO 76–77 GHz FMCW millimeter-wave radar unit.


* **Radar Field Coverage:** Delivers a $260\text{ m}$ localized omnidirectional target detection range with an independent horizontal field of view (FOV) of $\pm60^{\circ}$ and a 50 ms rapid refresh rate.


* **Local Gateway Management:** Includes an isolated weather-proof housing containing a local +24 VDC battery supply and an STM32WL or STM32F103 processing gateway. This local gateway ingests radar track frames over a localized 30-meter CAN/CAN FD bus and converts them into wireless telemetry packets.



### Non-Rotating Acoustic Nodes (3x Units Configured at 280m Radius)

Three static passive direction-finding arrays are interleaved between the radar nodes to triangulate airborne acoustic profiles.

* **Acoustic Array Sub-Frames:** Every node features 3 distinct, separated sub-frame sensor pods mounted to a central mounting frame ring collar.


* **Sensor Hardware Topology:** Each individual pod frame encloses an 8-point omnidirectional INMP441 MEMS microphone cluster paired with a $200\text{ mm}$ parabolic reflector dish to focus incoming signals passively.


* **Localized Signal Processing:** Powered by an onboard STM32F411 custom controller circuit that handles immediate passive Direction of Arrival (DoA) beamforming calculations before packaging data for wireless uplink transmission.



---

## 3. DEVICE POSITION MAP — FIELD COORDINATES

The coordinate tracking reference system establishes the Central EO Command Station ground level as the absolute point $(0, 0, 0)$.

| Node Identification | Physical Modality / Sensor Base | Target Sector Azimuth | Cartesian Location $(X, Y, Z)$ in Meters | Local Coverage Footprint |
| --- | --- | --- | --- | --- |
| **Center Master** | EO Sensor Pod Subsystem

 | $0^{\circ}$ (North Reference)

 | $(0.00, 0.00, 1.50)\text{ m}$<br> | $60^{\circ}$ Base FOV / $20\times$ Variable Zoom

 |
| **Radar Node #1** | CTLRR-220PRO mmWave Radar

 | $60^{\circ}$ Vector Azimuth | $(216.51, 125.00, 1.40)\text{ m}$ | $260\text{ m}$ Local Detection Radius

 |
| **Radar Node #2** | CTLRR-220PRO mmWave Radar

 | $180^{\circ}$ Vector Azimuth | $(-250.00, 0.00, 1.40)\text{ m}$ | $260\text{ m}$ Local Detection Radius

 |
| **Radar Node #3** | CTLRR-220PRO mmWave Radar

 | $300^{\circ}$ Vector Azimuth | $(125.00, -216.51, 1.40)\text{ m}$ | $260\text{ m}$ Local Detection Radius

 |
| **Acoustic Node #1** | 8x INMP441 Cluster Array

 | $0^{\circ}$ Vector Azimuth | $(0.00, 280.00, 1.20)\text{ m}$ | $200\text{ m}$ Passive DF Range

 |
| **Acoustic Node #2** | 8x INMP441 Cluster Array

 | $120^{\circ}$ Vector Azimuth | $(-140.00, 242.49, 1.20)\text{ m}$ | $200\text{ m}$ Passive DF Range

 |
| **Acoustic Node #3** | 8x INMP441 Cluster Array

 | $240^{\circ}$ Vector Azimuth | $(-140.00, -242.49, 1.20)\text{ m}$ | $200\text{ m}$ Passive DF Range

 |

---

## 4. CONSTELLATION DATA FLOW & NETWORK ARCHITECTURE

```
┌───────────────────────────────┐               ┌───────────────────────────────┐
│ DISTRIBUTED RADAR OUTPOSTS    │               │ DISTRIBUTED ACOUSTIC NODES    │
│ (3x Nodes @ 250m Radius)      │               │ (3x Nodes @ 280m Radius)      │
│                               │               │                               │
│ [CTLRR-220PRO Radar Module]   │               │ [8x INMP441 Mic Clusters]     │
│               │               │               │               │               │
│     [Local CAN / CAN FD Bus]  │               │     [Local I2S / SPI Lines]   │
│               ▼               │               │               ▼               │
│ [STM32WL / F103 Gateway Node] │               │ [STM32F411 Local Processor]   │
└───────────────┬───────────────┘               └───────────────┬───────────────┘
                │                                               │
    [Long-Range LoRa Wireless]                      [Long-Range LoRa Wireless]
    (868MHz / 915MHz Telemetry)                     (868MHz / 915MHz Telemetry)
                │                                               │
                └───────────────────────┬───────────────────────┘
                                        │
                                        ▼
                        ┌───────────────────────────────┐
                        │ MASTER CENTRAL COMMAND ENCL.  │
                        │                               │
                        │   [Master LoRa Base Station]  │
                        │               │               │
                        │        [Internal Ethernet]    │
                        │               ▼               │
                        │   [AI WORKSTATION CORE]       │
                        │   • Kalman Target Fusion      │
                        │   • RTSP Video Capture Stream │
                        │   • UART LRF Tracking Loop    │
                        │               │               │
                        │               ▼               │
                        │   [INDUSTRIAL OPERATOR C&C]   │
                        └───────────────────────────────┘

```

---

## 5. REVISED COMMUNICATION & SIGNAL SPECIFICATIONS

### CAN / CAN FD Datalink (Localized Internal Outpost)

* **Hardware Interface:** High-speed shielded twisted pair cable limited to a maximum length of 30 meters within each local radar tripod sub-structure.


* **Transmission Baudrate:** Configurable at 500 kbps for standard CAN or up to 1 Mbps during intensive CAN FD bursts.


* **Latency Budget:** $20\text{--}50\text{ ms}$ processing times per data cluster frame.


* **Termination:** Fixed $120\,\Omega$ termination resistors placed at both ends of the outpost bus architecture.



### LoRa Telemetry Link (Wide-Area Backhaul Network)

* **Frequency Allocation:** Operates on globally accepted license-free 868 MHz or 915 MHz ISM radio bands.


* **Operational Footprint:** Assured wireless performance spanning a $2\text{--}5\text{ km}$ structural Line-of-Sight (LOS) field layout.


* **Signal Parameters:** Fixed 125 kHz bandwidth using adaptive spreading factors spanning SF7 to SF12.


* **Data Integrity Profile:** Integrated ACK-based handshaking protocols featuring up to 3 automated packet retry attempts to minimize dropped tracks.



### Ethernet Network Protocol (Local Command Core)

* **Bandwidth Capacity:** Standard IEEE 8003 1 Gbps local area network switch integration.


* **Video Delivery Stream:** H.264/H.265 compressed High-Definition video delivered using real-time RTSP/RTP multicasting protocols.


* **Control Packets:** Low-latency UDP transport layers handle immediate PTZ steering joystick operations.


* **Telemetry Transport:** Background system health reporting and system diagnostic parameters are mapped over MQTT transport loops.



---

## 6. FIELD OPERATION & SYSTEM PERFORMANCE TARGETS

### Target Detection & Tracking Performance

* **Drone Detection Range:** Interleaved sensor configurations guarantee a operational tracking envelope of 100–500 meters with an absolute $50\text{ m}$ spatial detection precision margin.


* **Classification Accuracy:** Deep learning categorization loops secure an 85–95% accuracy rate for identifying moving drone profiles.


* **Multi-Target Capacity:** Real-time multi-sensor fusion processing tracks between 5 to 20 individual targets simultaneously.


* **System Latency:** Total end-to-end processing delay from initial remote physical target intersection to local operator display warning remains below 3 seconds.



### Environmental Durability Limits

* **Ingress Rating:** Remote node enclosures and mechanical sensor gimbals meet IP65 dust-tight and water-jet-resistant standards.


* **Temperature Tolerance:** Certified for full ambient temperature deployment ranges from $-10^{\circ}\text{C}$ up to $+50^{\circ}\text{C}$.


* **Wind Resistance:** Distributed low-profile tripod geometries prevent mechanical tipping or orientation errors up to a sustained crosswind threshold of 50 km/h.