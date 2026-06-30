# 阶段 A：测量链标定与验收

> 当前固件：连续短会话（5 hop/轮）+ `ranging_fence` 门控/滤波 + `ranging_publish` 统一发布语义。  
> 本文档为**可执行 checklist**，标定完成前勿进入围栏/告警（阶段 C）。

---

## 1. 当前门控/滤波参数（待标定）

| 宏 | 默认值 | 含义 | 标定后填写 |
|----|--------|------|------------|
| `FENCE_GATE_MIN_RSSI_DBM` | -92 | 低于此 RSSI 拒轮 | |
| `FENCE_GATE_MAX_RAW_SPREAD_RATIO` | 0.50 | 轮内 (max-min)/median 上限 | |
| `FENCE_GATE_MAX_JUMP_M` | 4.0 | 相对已发布值跳变上限 (m) | |
| `FENCE_FILTER_WINDOW` | 3 | 轮间中值滤波窗口 | |
| `RNG_CONTINUOUS_SAMPLE_COUNT` | 5 | 每轮 hop 数 | |
| `RNG_CONTINUOUS_SESSION_GAP_MS` | 1000 | 轮间隔 (ms) | |

---

## 2. 发布语义（`ranging_publish.h`）

| validity | 含义 | 屏上 Stat 建议 | `distance_m` 来源 |
|----------|------|----------------|-------------------|
| `RNG_PUBLISH_OK` | 本轮门控通过 | Done | `publishedDistance`（滤波） |
| `RNG_PUBLISH_HOLD` | 门控未过，沿用上次 OK | Hold | 上次 `publishedDistance` |
| `RNG_PUBLISH_RAW` | 尚无发布历史 | Done / Init | 本轮 `RngDistance` |
| `RNG_PUBLISH_INVALID` | 无有效距离 | Err / Init | — |

Watch / 日志建议每轮记录：

```
round, validity, round_m, pub_m, gate, rssi, samp, htx, age_ms
```

`gate` 枚举见 `RangingFenceGateReason_t`（0=OK，1=INVALID_STATUS，2=LOW_SAMPLES，3=BAD_RSSI，4=HIGH_SPREAD，5=JUMP）。

---

## 3. 测试矩阵

### 3.1 静态距离（各测 ≥ 30 min）

| 编号 | 真实距离 (m) | 环境 | 目标 σ (m) | 门控通过率 | 备注 |
|------|-------------|------|------------|------------|------|
| S1 | 1.0 | 视距 | ≤ 0.5 | ≥ 80% | |
| S2 | 3.0 | 视距 | ≤ 0.5 | ≥ 80% | 主标定点 |
| S3 | 5.0 | 视距 | ≤ 0.5 | ≥ 80% | |
| S4 | 10.0 | 视距 | ≤ 1.0 | ≥ 60% | 远距放宽 |

**记录项（每 5 min 打一次点）：** `round_distance_m`、`distance_m`、`validity`、`gate_reason`、`RSSI`、`Htx`。

### 3.2 长稳（固定 S2 = 3 m）

| 编号 | 时长 | 通过标准 |
|------|------|----------|
| L1 | 30 min | `Htx` 不持续爬升；无握手死锁 |
| L2 | 60 min | `published` 漂移 ≤ 0.5 m；Hold 比例可解释 |
| L3 | 60 min | 从机不断电，主机无 PER_ERROR 潮 |

### 3.3 动态 / 边界

| 编号 | 场景 | 观察 |
|------|------|------|
| D1 | 快速靠近 5 m → 1 m | JUMP 拒率、滤波滞后 (s) |
| D2 | 快速远离 1 m → 5 m | 同上 |
| D3 | 人体遮挡 30 s | HIGH_SPREAD / BAD_RSSI 比例 |
| D4 | 从机断电 10 s 再上 | 恢复时间 (轮数)、Htx 峰值 |
| D5 | 侧身 / 天线偏转 | 通过率 vs S2 基线 |

---

## 4. 单轮参数记录表（模板）

```
日期：
固件 / Git：
MASTER 板 SN：          SLAVE 板 SN：
场景编号：S__ / L__ / D__
真实距离 (m)：
环境：

--- 本轮 ---
round_index：
round_distance_m：
published_distance_m：
distance_m（对外）：
validity（0=INV 1=OK 2=HOLD 3=RAW）：
gate_reason：
gate_passed：
sample_count：
rssi_dbm：
handshake_tx (Htx)：
age_ms：
CntPacketRxOK / RngResultIndex：

--- 累计（本场景） ---
总轮数：
OK / HOLD / RAW / INV：
各 gate_reason 计数：
Htx>1 轮数：
distance_m 均值 / σ：
```

---

## 5. 阶段 A 完成标准（全部打勾再进阶段 B）

- [x] `ranging_core` 已删除，main 仅 `demoRanging` → `ranging_fence` → `ranging_publish` 一条门控链
- [ ] 屏显 / Watch 的 `Dist` 与 `RangingPublish.distance_m` 一致
- [ ] Hold 时 Stat 显示 Hold，不再假 Done
- [ ] S2 静态 30 min：σ ≤ 0.5 m（或项目组自定）
- [ ] L2 长稳 60 min：无 Htx 失控、无从机失联
- [ ] 参数表 6 项均有标定依据与推荐值
- [ ] D1–D5 至少各跑 1 次并记录

---

## 6. 阶段 B 入口（A 完成后）

1. 显示 / UART / 告警 **只读** `RangingPublishGet()`  
2. 从 `demoRanging.c` 抽出 `ranging_session`（连续会话、IRQ 掩码、握手重试）  

---

## 7. 常见问题对照

| 现象 | 可能原因 | 查什么 |
|------|----------|--------|
| Dist 不变，RSSI 变 | 门控拒，Hold 旧发布 | `validity=HOLD`，`gate_reason` |
| Htx 逐轮 +1 | 握手重试 | 从机同步、session gap |
| 长期全 Hold | 跳变/离散度阈値过严 | D1/D3，`FENCE_GATE_MAX_*` |
| 首轮一直 RAW | 滤波窗未满 | 正常，等 3 轮 OK |
