# SX1280 Ranging on HC32L021

Semtech SX1280 LoRa 测距固件，运行于华大 HC32L021C8PB。双板（MASTER + SLAVE）配合使用。

## 快速开始

1. 用 Keil µVision 打开 **`project/MDK/sx1280_ranging.uvprojx`**
2. 在 `sx1280/app/main.c` 中设置角色：
   - `DEMO_SETTING_ENTITY  DEMO_ROLE_SLAVE` → 编译烧录从机，先上电
   - `DEMO_SETTING_ENTITY  DEMO_ROLE_MASTER` → 编译烧录主机
3. 主机上电后延时 3 s 开始连续测距；Keil Watch 观察 `DemoResults.RngDistance`

详细说明见 [`sx1280/README.md`](sx1280/README.md)。

## 仓库结构

```
sx1280_test/
├── sx1280/              固件源码（唯一维护入口）
│   ├── app/             main、ranging/、lcd/
│   ├── driver/          Semtech SX1280 驱动
│   ├── hal/hc32/        HC32 平台移植
│   └── docs/            标定文档
├── project/             Keil 工程
│   ├── MDK/             ← 打开 sx1280_ranging.uvprojx
│   └── source/          ddl_device.h（HC32 DDL 必需）
├── driver/              HC32L021 DDL 外设库
├── mcu/common/          启动、中断、板级
└── ChangeLog.md         HC32 DDL 版本变更记录
```

## 数据流（调试）

```
demoRanging  →  ranging_fence  →  ranging_publish  →  ranging_display / Watch
```

## 文档

- [固件说明与引脚](sx1280/README.md)
- [阶段 A 标定 checklist](sx1280/docs/RANGING_PHASE_A.md)
