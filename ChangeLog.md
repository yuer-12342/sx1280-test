# Update History
------

## Rev1.1.0  Sep. 15, 2025

### documents
-  **更新hc32l021_ddl.chm文件**

### drivers
-  **ddl.h**
  - 添加驱动库版本号
-  **adc.h & adc.c**
  - 修改ADC_IBAS_Current宏定义值和ADC_SqrStcInit()初始化值
  - 修改ADC_Trigger_Src_Select宏定义中对应触发源的注释
  - 修改ADC_SqrInit()和ADC_Enable()中ADC使能后延时时长
-  **atim3.h & atim3.c**
  - 修改ATIM3_Mode0_Tog_ON_OFF组宏定义中注释、修改ATIM3_Mode23_UND_Repeat_Cnt组宏定义中注释
  - 修改ATIM3_Mode1_InputConfig()赋值方式
  - 删除DMA相关宏定义、其它注释、文档相关值修改
-  **flash.c**
  - 修改FLASH_ContinousWriteByte()、FLASH_ContinousWriteHalfWord()和FLASH_ContinousWriteWord()连续模式中OP值
-  **gtim.h**
  - 修改GTIM_ETP_Extern_Input_Filter宏定义组
-  **iwdt.h**
  - 修改IWDT_Init()中IWDT_SR_PRSF_Msk为IWDT_SR_CRF_Msk
-  **lpuart.h & lpuart.c**  
  - 修改LPUART_Init()，涵盖使用XTL生成9600和4800的设置，单线半双工设置
  - 修改LPUART_TransmitPoll()LPUART_TransmitPollTimeOut()中TC判断
  - 修改LPUART_ReceiveInt()为LPUART_DataReceive()、LPUART_TransmitInt()为LPUART_DataTransmit()且移除其中对TXE为空的判断
  - LPUART_ReceivePoll()和LPUART_ReceivePollTimeOut()中添加FE和PE判断并返回Error
  - 添加LPUART_IntEnableGet()
  - 修改LPUART_StcInit()
  - 修改LPUART_MultiModeConfig()形参名和赋值方法
  - 删除LPUART_Bit8Set()和LPUART_Bit8Clear()
  - stc_lpuart_init_t结构体中添加u32BaudRateGenSelect
  - 修改宏定义LPUART_B8_MULTI_DataOrAddr为LPUART_B8_MULTI_DATA_OR_ADDR、修改宏定义LPUART_XTL_BAUD_NORMAL为LPUART_BAUD_NORMAL、修改宏定义LPUART_FLAG_ALL为LPUART_FLAG_CLR_ALL
  - 添加LPUART_D8_Select宏定义组
  - 修改LPUART_Trans_Mode_Select宏定义组，添加LPUART_MODE_HD_TX_RX
  - 其它注释、文档相关值修改
-  **sysctrl.h & sysctrl.c**
  - 修改PeriphResetLpaurt1为PeriphResetLpuart1
  - 修改SYSCTRL_DEBUG_ATIMER3_ACTIVE为SYSCTRL_DEBUG_ATIM3_ACTIVE
  - 修改RCL trim地址为32位
-  **vc.h & vc.c**  
  - 添加VC trim功能
      - 修改VC_Init()，添加VC trim设置
      - 添加VC_TrimSet()
      - stc_vc_init_t结构体中添加u8TrimSetSelect、修改VC_StcInit()
      - 添加VC_Trim_Select宏定义组、VC trim值地址
  - 添加DAC启动后延时    
-  **其它**
  - 注释修改

### example
-  **adc**
  - 添加adc_scan_sqr_avcc样例，实现通过VCAP反推AVCC
  - 修改adc_scan_sqr_ctrim_trig样例
  - 修改因ADC_IBAS_Current宏定义变化相关的赋值
  - 因lpuart驱动库修改而更改adc_scan_sqr_lpuart_trig样例
-  **atim3**
  - 修改atimer3_m1_pwc样例和atimer3_m1_pwc_oneshot样例中判断语句错误
  - 修改atimer3_m23_pwm_adc样例中因ADC_IBAS_Current宏定义变化相关的赋值
  - 修改atimer3_m23_pwm_vc_brake样例中VC的初始化函数
  - 修改注释和变量名等
-  **flash**
  - 修改ram_code.sct中地址值
-  **hsi2c**
  - 修改中断配置调用函数为EnableNvic()
  - 添加hsi2c_mcu_slave_lpm样例
-  **stk_test**
  - 因lpuart驱动库修改而更改stk_test样例
  - 修改PUTCHAR_PROTOTYPE实现
-  **lpuart**
  - 样例实现修改
  - 修改PUTCHAR_PROTOTYPE实现
-  **vc**
  - 因添加VC trim功能修改初始化设置
  - vc_vref_1p5_falling_int样例名修改为vc_vref_vcap_falling_int样例
-  **trng**
  - 样例变量名修改
-  **其它**
  - 注释修改

### mcu
- **common**
  - 头文件hc32l021.h更新
  - 修改HC32L021.svd和HC32L021.sfr

------
## Rev1.0.0  Apr. 8, 2025

### documents
-  **更新hc32l021_ddl.chm文件**

### drivers
-  **atim.h & atim.c**
  - 修改ATIM3_Mode23_CHxA_Match_INT、ATIM3_Mode23_CHxB_Match_INT、ATIM3_Mode23_UND_Repeat_Cnt和ATIM3_Mode23_OVF_Repeat_Cnt的宏定义和调用
-  **ctrim.h & ctrim.c**
  - 修改ctrim.h注释
  - 修改CTRIM_REF_Clock_Div_Select的宏定义
  - 修改CTRIM_CalibStcInit()和CTRIM_TimerStcInit()函数
-  **lpuart.c**
  - 删除LPUART_Init()多余代码
-  **sysctrl.h & sysctrl.c**
  - 删除SYSCTRL_RCL_Mode_Select宏定义
  - 删除SYSCTRL_RCLModeSelect()函数定义和申明
  - sysctrl.h中，SPI0和HSI2C0相关定义修改为SPI和HSI2C

### example
-  **startup_hc32l021.s**
  - SPI0和HSI2C0的中断相关定义修改为SPI和HSI2C
-  **atim3**
  - 修改atimer3部分样例readme
  - 修改因宏定义变化相关的调用
-  **hsi2c**
  - HSI2C0的模块名和中断相关定义修改为HSI2C
-  **lvd**
  - 修改lvd_detect_reset样例中main函数中判断语句
-  **spi**
  - SPI0的模块名和中断相关定义修改为SPI

### mcu
- **common**
  - interrupts_hc32l021.c中, SPI0和HSI2C0的中断相关定义修改为SPI和HSI2C
  - 头文件hc32l021.h中，SPI0和HSI2C0的模块名和中断相关定义修改为SPI和HSI2C
  - 修改HC32L021.svd和HC32L021.sfr

------
## Beta1.2.0  Mar 18, 2025

### documents
-  **更新hc32l021_ddl.chm文件**

### drivers
-  **各模块驱动**
  - 修改lpuart.h、iwdt.h和rtc.h注释
-  **atim.h**
  - 修改ATIM3_Mode23_Filter的宏定义
-  **flash.h & flash.c**
  - 修改条件编译中编译器版本号  
-  **gpio.h**
  - 修改和GPIOAUX相关的宏定义
-  **vc.h**
  - 修改空白窗口源选择源宏定义
-  **spi.h**
  - 修改SPI中断标志位宏定义
-  **sysctrl.h & sysctrl.c**
  - 修改和头文件相关的宏定义和枚举常量
  - 修改SYSCTRL_ClockSrcEnable()中时钟使能宏定义值

### example
-  **各模块样例**
  - 修改adc、atimer3、ctimer、ctrim、gpio、iwdt、rtc、spi、trng、stk_test和hsi2c部分样例readme和注释
-  **adc**
  - 修改adc_scan_sqr_hw_trig样例中读取转换结果代码
-  **ctimer**
  - 修改gtimer_cascade_timer样例Atim3Config()定义和调用、修改btimer_toggle样例Btim3Config()函数
  - 修改gtimer_capture_rising样例变量定义和赋值顺序
-  **iwdt**
  - 删除样例中IWDT_Start()调用
-  **spi**
  - 添加主从机单线半双工和单工通信样例
  - 修改spi_slave_poll样例中片选信号判断  

### mcu
- **common**
  - 修改board_stkhc32l021.c中UID地址
  - 修改头文件hc32l021.h
  - 修改HC32L021.svd和HC32L021.sfr

------
## Beta1.1.0  Feb 24, 2025

### documents
- **更新hc32l021_ddl.chm文件**

### drivers
-  **各模块**
   - 添加结构体初始化函数
   - 修改和头文件相关的宏定义
   - 修改部分模块的注释问题
-  **gpio.c**
  - GPIO_Init()调整模拟数字功能的设置顺序和初始电平设置顺序
-  **rtc.c**
  - 添加RTC_CompValueCalculate()函数
-  **iwdt.c**
  - 添加对WWDT功能的处理
-  **hsi2c.c & hsi2c.h**
  - 删除Doze、MCFGR0相关功能代码
  - 修改宏定义命名
  - 修改HSI2C_MasterStatusMachineSubAddr()函数
  - 删除HSI2C_MasterMcfgr0Config()函数
  - 修改HSI2C_SlaveTransAbort()函数
  - 增加主机对于ALF和PLTF错误处理
-  **sysctrl.c & sysctrl.h**
  - 修改宏定义命名
  - 增加XTL驱动能力宏定义
  - 修改SYSCTRL_DebugActiveEnable()函数
  - 修改SYSCTRL_DebugActiveDisable()函数
-  **vc.h**
  - 修改宏定义命名
-  **trng.c**
  - 修改宏定义命名
-  **flash.c & flash.h**
  - 添加FLASH Readonly使能和禁止API
-  **lpuart.c & lpuart.h**
  - 修改初始化函数中波特率计算
  - 修改宏定义命名
  
### example
-  **各模块样例**
   - 修改初始化结构体清零为调用结构体初始化函数
   - 修改部分样例的注释问题
-  **adc**
   - 添加轮询样例
-  **ctrim**
   - 修改ctrim_rc48m_cal样例代码中trim值处理
-  **flash**
   - 添加擦写操作前解除FLASH Read Only处理
-  **gpio**
   - gpio_deepsleep_wakeup样例删除按键进入DeepSleep操作
-  **hsi2c**
  - 增加主机对于ALF和PLTF错误处理
-  **iwdt**
  - 增加DeepSleep下IWDT功能样例和WWDT功能样例
-  **lpuart**
  - 修改中断标志位清除调用函数
-  **lvd**
   - 修改样例中端口配置为模拟端口
-  **rtc**
  - 添加RTC_CompValueCalculate()调用
-  **spi**
  - 修改从机样例中端口配置方向
  
### mcu
- **common**
   - 添加结构体初始化函数
   - 修改和头文件相关的宏定义  
   - 修改interrupts_hc32l021.h宏定义命名
   - 修改头文件hc32l021.h
   - 修改HC32L021.svd和HC32L021.sfr
   - system_hc32l021.h中SystemInit()添加FLASH Read Only配置

------
## Beta1.0.0  Jan 5, 2025
- Initial release.
