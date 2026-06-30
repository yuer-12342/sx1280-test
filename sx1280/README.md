# SX1280 Ranging Demo (HC32L021)

Semtech 官方 SX1280 ranging 示例移植到 HC32L021C8PB。**Semtech 驱动/应用尽量不改**，平台相关代码集中在 `hal/hc32/`。

Keil 工程：`project/MDK/sx1280_ranging.uvprojx`

## 目录结构

```
sx1280/
├── app/
│   ├── main.c              入口：角色、连续/按键模式、主循环
│   ├── ranging/            测距核心（Semtech demo + 门控 + 发布）
│   │   ├── demoRanging.c/h     MASTER/SLAVE 状态机、握手、hop 测距
│   │   ├── ranging_fence.c/h   样本门控 + 3 轮中值滤波
│   │   ├── ranging_publish.c/h 对外 OK/HOLD/RAW/INVALID
│   │   └── FreqLUT.h           跳频 LUT（当前 DEMO_FREQ_HOPPED_RANGING=0 未用）
│   └── lcd/
│       ├── lcd*.c/h        ST7735S 驱动
│       └── ranging_display.c/h  测距 UI（读 RangingPublishGet）
├── driver/                 Semtech sx1280-driver-c（sx1280.c 等，勿改）
│   ├── RangingCorrection.h
│   ├── rangingCorrection_tables.c
│   └── rangingCorrection/  SF×BW 校正 LUT 头文件
├── hal/hc32/               HC32 平台层（仅改这里）
│   ├── sx1280_hal_hc32.*   SPI / GPIO / BUSY / DIO1
│   ├── shared_spi_bus.*    SX1280 + TFT 共用 SPI
│   ├── board_ui.*          PA12 按键、PA09 蜂鸣器
│   └── ...
└── docs/
    └── RANGING_PHASE_A.md  标定 checklist
```

## 模块职责

| 模块 | 职责 | 调试入口 |
|------|------|----------|
| `demoRanging` | 射频测距会话、握手、hop 采样 | `DemoResults`, `DemoInternalState`, `DemoSettings.RngStatus` |
| `ranging_fence` | RSSI/离散度/跳变门控，轮间滤波 | `RangingFenceGetOutput()` → `gateReason` |
| `ranging_publish` | 统一对外距离与有效性 | **`RangingPublishGet()`**（屏/UART/告警应只读此 API） |
| `ranging_display` | TFT 刷新 | `Dist` 与 `RangingPublish.distance_m` 一致 |

## 配置宏（`app/main.c`）

| 宏 | 默认 | 说明 |
|----|------|------|
| `DEMO_SETTING_ENTITY` | `MASTER` | 改为 `DEMO_ROLE_SLAVE` 烧从机 |
| `DEMO_RNG_CONTINUOUS_MODE` | `1` | `0` = 按键触发，每轮 30 hop |
| `DEMO_USE_TFT` | MASTER 自动开 | 从机无屏 |

测距参数（`app/ranging/demoRanging.h`）：`DEMO_FREQ_HOPPED_RANGING=0` 单信道 2450 MHz。

## 引脚

| 信号 | 引脚 |
|------|------|
| SX1280 CS / SPI | PA00 / PA01–03 |
| BUSY / NRST / DIO1 | PA05 / PA08 / PA15 |
| TFT CS / DC / RES / BLK | PA04 / PA07 / PA10 / PA11 |

SPI 6 MHz Mode 0；SX1280 与 TFT 独立 CS，`spi_mutex` 互斥。

## MASTER / SLAVE 烧录

1. `main.c` → `DEMO_ROLE_SLAVE`，编译烧录从机并运行。
2. 改为 `DEMO_ROLE_MASTER`，编译烧录主机；上电 3 s 后开始测距。
3. Watch：`DemoResults.RngDistance`、`RangingPublishGet()->distance_m`、`RangingPublishGet()->validity`。

## 中断与轮询

- DIO1 (PA15) → PORTA 中断 → `SX1280OnDioIrq()`
- 轮询模式：`RangingDemoRun()` / `RangingDemoPollRadio()` 内 `SX1280ProcessIrqs()`
- SysTick 1 ms → Semtech 软件定时器

## 标定

[`docs/RANGING_PHASE_A.md`](docs/RANGING_PHASE_A.md)
