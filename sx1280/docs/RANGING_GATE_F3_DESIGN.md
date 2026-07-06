# F3 趋势门控 — 实现与验收对照

> 稳定判区 + Dist 跟随变距 + 识别真实移动 vs 异变

## 代码文件

| 文件 | 状态 |
|------|------|
| `ranging_fence.c/h` | L1/L2/L3 三层门控 |
| `ranging_publish.c/h` | `RNG_PUBLISH_TRACK` |
| `ranging_display.c` | Stat=Trk，Gate=Trk/Wait/Ano |
| `fence_alarm.c` | `USE_RND=0`，OK+Trk 报警 |

## 屏显

| Stat | Gate | 含义 |
|------|------|------|
| Done | OK | L2 稳态 |
| Trk | Trk | L3 趋势跟踪，Dist 更新 |
| Hold | Wait/Sprd/Ano/Jump | 未更新 Dist |

## 参数（`ranging_fence.h`）

| 宏 | 值 |
|----|-----|
| `FENCE_GATE_STATIC_DELTA_M` | 0.8 m |
| `FENCE_GATE_SPREAD_RATIO_FAR` | 0.65 |
| `FENCE_TRACK_CONFIRM_ROUNDS` | 2 |
| `FENCE_TRACK_CREEP_FAR_MAX_M` | 1.5 m/轮 |
| `FENCE_TRACK_CREEP_NEAR_MAX_M` | 1.0 m/轮 |

## 验收（见 RANGING_ZONE_DEMO_PLAN.md）

| ID | 通过线 |
|----|--------|
| S | 3 m，Done+Trk 的 Rnd/Dist 极差 ≤1.5 m |
| D1 | 3→6 m 拉远，≤20 s Dist 进 4～7 m，出现 Trk |
| Z1 | 独立上电，M_mid−M_near ≥1.5 m |
| A2 | Dist>Set → OUT! |
