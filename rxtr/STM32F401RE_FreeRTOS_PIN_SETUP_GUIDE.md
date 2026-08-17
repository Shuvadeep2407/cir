# STM32F401RE — FreeRTOS "Main EO" Pin & Setup Guide

Board: **NUCLEO-F401RE** (LQFP64, STM32F401RET6, 512 KB FLASH, 96 KB RAM)
Toolchain: **STM32CubeIDE** + **STM32CubeMX 6.18.1**
RTOS: **FreeRTOS (CMSIS‑RTOS v2 wrapper)** — the free RTOS used by the project.
Project skeleton used to build this guide: `e:\cir-main\rxtr\main eo\` (project name `401reeth`).

> **IMPORTANT — read before wiring**
> Your original plan said **W5500 on SPI1** and **CC1101 on SPI2**.
> The **actual configured project** uses the **opposite/mixed** layout:
> - **CC1101 radio is wired to SPI1** (PA5/PA6/PA7) with its chip‑select on **PB6**.
> - **W5500 Ethernet is NOT yet driven in code.** SPI3 (PC10/PC11/PC12) is mapped in CubeMX but there is **no W5500/ETH driver** anywhere in `main.c`.
>
> Follow the table below (what is actually wired and compiled) and see the
> "W5500 note" at the end if you want to add Ethernet.

---

## 1. FreeRTOS Setup (one‑time in CubeMX)

1. Open `401reeth.ioc` in STM32CubeMX.
2. **Middleware and Software Packs → FreeRTOS** → Interface **CMSIS_V2**.
3. Set the following (already done in this project):
   - `configTOTAL_HEAP_SIZE` = **16384** (bytes; FreeRTOS heap_4)
   - `configCHECK_FOR_STACK_OVERFLOW` = **2**
   - `configUSE_NEWLIB_REENTRANT` = **1**  → gives you working `snprintf/printf`
   - Timers / Mutexes / Semaphores = enabled.
4. **Project Manager → Advanced settings**: timebase = **TIM5** (NOT SysTick, because FreeRTOS owns SysTick).
5. Generate code, open in STM32CubeIDE, **Build** (`Ctrl+B`).

Build output lives in `main eo\Debug\401reeth.elf`.

### Tasks / threads created (in `main.c` `MX_FREERTOS_Init`)
| Task          | Priority     | Stack  | Role |
|---------------|--------------|--------|------|
| `Task_MainEO` | AboveNormal  | 2048 B | Hub logic, USB host, forwards data to PC |
| `Task_CC1101` | High         | 1024 B | Reads CC1101 radio (remote acustick/radar/jammer) |
| `Task_PCComm` | Normal       | 1024 B | PC commands via USART2 / ETH |

Queues: `xSensorQueue` — 20 slots of `uint16_t`.

> USB Host library also spawns its own thread (`USBH_PROCESS_PRIO` = AboveNormal, stack 1024 B).

---

## 2. Pin Map (as actually configured)

All GPIO speed/pull settings come from the `.ioc`. **Bold** items are user GPIO labels.

### Power / Debug
| Pin      | Function | Notes |
|----------|----------|-------|
| PH0/PH1  | HSE 8 MHz | External clock source (PLL → 84 MHz) |
| PC14/PC15| LSE 32.768 kHz | RTC |
| PA13/PA14| SWDIO / SWCLK | Serial Wire debug |

### USB Host (camera)
| Pin  | Function | Notes |
|------|----------|-------|
| PA11 | USB_OTG_FS_DM | Host only, embedded PHY |
| PA12 | USB_OTG_FS_DP | Host only |
| PA15 | GPIO output | available |

Host class = **HID** (`VirtualModeFS=Hid`), `USBH_USE_OS=1`.

### UART 1 — 30× zoom lens
| Pin  | Function |
|------|----------|
| PA9  | USART1_TX |
| PA10 | USART1_RX |

### UART 2 — USB‑TTL to PC
| Pin | Function | Label |
|-----|----------|-------|
| PA2 | USART2_TX | **USART_TX** |
| PA3 | USART2_RX | **USART_RX** |

> Main debug/command console (`HAL_UART_Transmit(&huart2, ...)`).

### UART 6 — LRF (laser range finder)
| Pin | Function |
|-----|----------|
| PC6 | USART6_TX |
| PC7 | USART6_RX |

### I²C 1 — compass
| Pin | Function |
|-----|----------|
| PB8 | I2C1_SCL |
| PB9 | I2C1_SDA |

### SPI 1 — CC1101 radio (433 MHz)  ★ actual layout
| Pin | Function |
|-----|----------|
| PA5 | SPI1_SCK |
| PA6 | SPI1_MISO |
| PA7 | SPI1_MOSI |
| **PB6** | **c1101_cs** (GPIO output, CS) |

`SPI_BAUDRATEPRESCALER_64`, full‑duplex master. This receiver's ID = `DEVICE_ID 11` (0x0B).
Remote nodes listened for: **22, 33, 44** (acustick), **55, 66, 77** (radar), **88** (jammer).

### SPI 3 — mapped but currently unused
| Pin  | Function |
|------|----------|
| PC10 | SPI3_SCK |
| PC11 | SPI3_MISO |
| PC12 | SPI3_MOSI |

No driver uses `hspi3` yet — natural home if you add the **W5500** here.

### I2S 2 (uses SPI2 pins) — configured, currently unused
| Pin  | Function | Mode |
|------|----------|------|
| PB10 | I2S2_CK | Half duplex master |
| PB12 | I2S2_WS | Half duplex master |
| PB15 | I2S2_SD | Half duplex master |

### Spare GPIO (addressed/relay outputs)
| Pin | Label |
|-----|-------|
| PC4 | **a1** |
| PC5 | **b1** |
| PB0 | **a2** |
| PB1 | **b2** |

---

## 3. Clock Summary (from RCC)
- HSE = 8 MHz → PLLM=4 → 2 MHz → PLLN=168 → 336 MHz VCO → **PLLP=4 → SYSCLK 84 MHz**
- AHB = 84 MHz · APB1 = 42 MHz (USART2, I2C, SPI2/I2S2) · APB2 = 84 MHz (USART1, SPI1, USART6, SPI3)
- 48 MHz USB clock = PLLQ.

---

## 4. W5500 note (typical addition if you want Ethernet)
- W5500 is an SPI slave with its own CS (e.g. **PB12** or a free GPIO like **PA15**).
- On **SPI1** it would share the bus with CC1101 (both can't be selected simultaneously), so the safer home is the **free SPI3 (PC10/11/12)** with a separate CS pin.
- Add `W5500_SPI_Read/Write` helpers using `HAL_SPI_TransmitReceive(&hspi3, ...)`, mirroring the existing `cc1101_read_burst`.
- The "always‑on connection" forwarding to PC is handled by the `Task_PCComm` thread; only the transport (UART2 vs ETH) changes.

---

## 5. Build / Flash / Debug
1. STM32CubeIDE → open project `401reeth`.
2. `Project → Build All`.
3. Launch config `401reeth Debug.launch` (ST‑LINK, SWD, connect‑under‑reset), CPU 84 MHz.
4. On boot you should see on the UART2 console:
   - `CC1101 Detected. Version: 0x14` (or similar),
   - then RX packet lines from acustick/radar/jammer nodes.

---

## 6. FAQ / common gotchas
- **`snprintf`/`printf` crash** → ensure `configUSE_NEWLIB_REENTRANT=1` (it is) and, if using floats, add `-u _printf_float`.
- **USB not starting** → USB Host must be initialized inside `Task_MainEO` (after scheduler starts), not before `osKernelStart()`. Your `MX_USB_HOST_Init()` runs in `StartMainEOTask`.
- **Stack overflow** → `configCHECK_FOR_STACK_OVERFLOW=2` is on; watch `vApplicationStackOverflowHook` (defined in `freertos.c` — currently empty, add a breakpoint there).
- **CC1101 RX FIFO errors** → code automatically flushes the FIFO on overflow.
