/**
 * @file app_param_dict.h
 * @brief Global parameter dictionary definition and basic access APIs.
 *
 * 本模块对应“模块2：全局数据字典”的基础骨架，只实现
 * - 参数描述结构体
 * - 少量示例参数
 * - 只读/读写属性与数值边界检查
 * - 通过 ID 查找与安全写入接口
 *
 * 后续可在不修改接口的前提下持续向表中追加参数。
 */

 #ifndef APP_PARAM_DICT_H
 #define APP_PARAM_DICT_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdint.h>
 #include <stdbool.h>
 
 /**
  * @brief Parameter access permission.
  */
 typedef enum
 {
   PARAM_ACCESS_RO = 0,  /**< Read only, cannot be modified from HMI. */
   PARAM_ACCESS_RW = 1   /**< Read / write. */
 } ParamAccess_t;
 
 /**
  * @brief Parameter value encoding / type.
  *
  * 仅列出当前明确会用到的几种；后续可按需追加。
  */
 typedef enum
 {
   PARAM_TYPE_UINT8 = 0,      /**< 0 ~ 255              (uint1)      */
   PARAM_TYPE_INT8,           /**< -128 ~ 127           (sint1)      */
   PARAM_TYPE_UINT16,         /**< 0 ~ 65535                        */
   PARAM_TYPE_INT16,          /**< -32768 ~ 32767                   */
   PARAM_TYPE_INT32,          /**< 通用 32 位有符号               */
   PARAM_TYPE_UINT32,         /**< 通用 32 位无符号               */
   PARAM_TYPE_SCALED_1000_I32 /**< 以 1000 为缩放因子的定点数，如 1000uint4 */
 } ParamType_t;
 
 /**
  * @brief Logical parameter ID.
  *
  * 这里的 ID 建议与协议或 PDF 中的“参数地址”保持一一对应，
  * 也可以单独维护逻辑 ID 和物理地址映射。
  */
 typedef uint16_t ParamId_t;
 
 /**
  * @brief Parameter descriptor.
  *
  * 注意：为兼容多种类型，min/max/value 使用统一的 int32_t 存储，
  * 由 @ref ParamType_t 决定如何解释。
  */
 typedef struct
 {
   ParamId_t     id;          /**< 逻辑参数 ID / 地址 */
   ParamType_t   type;        /**< 编码 / 数据类型   */
   ParamAccess_t access;      /**< 读写属性          */
   int32_t       min_value;   /**< 最小值 (扩展为 int32 存储) */
   int32_t       max_value;   /**< 最大值 (扩展为 int32 存储) */
   uint8_t       scale;       /**< 小数位数，例如 0,1,2,3；仅用于 UI 显示 */
   int32_t       value;       /**< 当前值（同样按 type 解释） */
 } ParamDef_t;
 
 /**
  * @brief 获取全局参数表指针及表长度。
  *
  * 用于遍历或调试，不建议在业务逻辑中频繁调用。
  */
 const ParamDef_t *APP_ParamDict_GetTable(uint16_t *outCount);
 
 /**
  * @brief 通过参数 ID 查找参数描述。
  * @param id       逻辑参数 ID
  * @return         找到时返回指针，否则返回 NULL
  */
 ParamDef_t *APP_ParamDict_FindById(ParamId_t id);
 
 /**
  * @brief 读取参数当前值（只读封装，避免外部直接访问结构体）。
  * @param id       参数 ID
  * @param outVal   输出值指针
  * @return         true 成功，false 未找到
  */
 bool APP_ParamDict_GetValue(ParamId_t id, int32_t *outVal);
 
 /**
  * @brief 在边界和权限检查后尝试更新参数值。
  *
  * - 若参数不存在，返回 false
  * - 若为只读参数，返回 false
  * - 若超出 min/max 范围，返回 false
  *
  * @param id       参数 ID
  * @param newVal   新值（按 ParamType_t 的解释编码）
  * @return         true 更新成功，false 失败
  */
 bool APP_ParamDict_TrySetValue(ParamId_t id, int32_t newVal);

/**
 * @brief 内部强制写值（跳过读写权限检查），仍保留边界检查。
 *
 * 用于 UART 回包等设备状态同步场景，允许更新只读参数。
 */
bool APP_ParamDict_SetValueUnsafe(ParamId_t id, int32_t newVal);
 
 /** 一些示例参数 ID，后续可替换为协议中的正式定义 **/
 enum
 {
   PARAM_ID_WORK_MODE      = 1,  /**< 当前工作模式 */
   PARAM_ID_NET_IP_ADDR    = 2,  /**< 网口 IP 地址（可按分段或索引化存储） */
   PARAM_ID_UART_BAUDRATE  = 3,  /**< 串口波特率   */
  PARAM_ID_ENV_TEMPERATURE = 4, /**< 环境温度 (状态量，只读示例) */

  PARAM_ID_ROUTING_PROTOCOL = 10, /**< 路由协议：0=OLSR,1=AODV,2=BATMAN 等 */
  PARAM_ID_ACCESS_PROTOCOL  = 11, /**< 接入协议：当前仅 0=TDMA */
  PARAM_ID_DEVICE_ID        = 12, /**< 设备 ID（来自 0x05） */

  /* 0x05 设备基础信息 */
  PARAM_ID_GPS_LONGITUDE   = 20, /**< 经度（协议原始缩放值） */
  PARAM_ID_GPS_LATITUDE    = 21, /**< 纬度（协议原始缩放值） */
  PARAM_ID_GPS_ALTITUDE    = 22, /**< 高度 */
  PARAM_ID_SAT_LOCK        = 23, /**< 卫星锁定状态 */

  /* 0x06 流量统计 */
  PARAM_ID_ETH_TX_CNT      = 30,
  PARAM_ID_ETH_RX_CNT      = 31,
  PARAM_ID_VOICE_TX_CNT    = 32,
  PARAM_ID_VOICE_RX_CNT    = 33,

  /* 0x07 自检状态 */
  PARAM_ID_BATTERY_CAP     = 40, /**< 电池余量(%) */
  PARAM_ID_FAN_STATE       = 41, /**< 风机状态 */
  PARAM_ID_SELFTEST_STATE  = 42, /**< 自检总状态 */
  PARAM_ID_CLOCK_SELECTION = 43, /**< 时钟选择状态 */
  PARAM_ID_ADC_STATUS      = 44, /**< ADC 状态 */
  PARAM_ID_CLOCK_SRC_TEMP  = 45, /**< 时钟源温度 */
  PARAM_ID_FREQ_WORD_CNT   = 46, /**< 频率字下发计数 */
  PARAM_ID_COMM_SENSE_ST   = 47, /**< 通信感知状态 */

  /* 0x08 开机显示参数 */
  PARAM_ID_TIME_HOUR       = 60,
  PARAM_ID_TIME_MINUTE     = 61,
  PARAM_ID_TIME_SECOND     = 62,
  PARAM_ID_NET_JOIN_STATE  = 63,
  PARAM_ID_WAVEFORM_GEAR   = 64,
  PARAM_ID_SIGNAL_BANDWIDTH= 65,
  PARAM_ID_FREQ_HOP_MODE   = 66,
  PARAM_ID_FIXED_FREQ      = 67,
  PARAM_ID_ADAPT_FREQ1     = 68,
  PARAM_ID_ADAPT_FREQ2     = 69,
  PARAM_ID_ADAPT_FREQ3     = 70,
  PARAM_ID_ADAPT_FREQ4     = 71,
  PARAM_ID_SPATIAL_FILTER  = 72,
  PARAM_ID_SYNC_MODE       = 73,
  PARAM_ID_TX_POWER        = 74,
  PARAM_ID_TX_POWER_ATTEN  = 75,
  PARAM_ID_MCS             = 76,
  PARAM_ID_SLOTLEN         = 77,

  /* 0x09 邻居信息 */
  PARAM_ID_NEIGHBOR_COUNT  = 80,
  PARAM_ID_NEIGHBOR_RSSI   = 81, /**< 最近邻/首邻 RSSI */
  PARAM_ID_NEIGHBOR_NODE_ID    = 82,
  PARAM_ID_NEIGHBOR_NODE_IP    = 83,
  PARAM_ID_NEIGHBOR_NODE_HOPS  = 84,
  PARAM_ID_NEIGHBOR_NODE_DELAY = 85,
  PARAM_ID_NEIGHBOR_NODE_LON   = 86,
  PARAM_ID_NEIGHBOR_NODE_LAT   = 87,
  PARAM_ID_NEIGHBOR_NODE_ALT   = 88,

  /* 0x0A 节点详细信息 */
  PARAM_ID_DETAIL_MEMBER_ID    = 100,
  PARAM_ID_DETAIL_SPATIAL      = 101,
  PARAM_ID_DETAIL_IP           = 102,
  PARAM_ID_DETAIL_CH1_HOP      = 103,
  PARAM_ID_DETAIL_CH1_FREQ     = 104,
  PARAM_ID_DETAIL_CH1_BW       = 105,
  PARAM_ID_DETAIL_CH1_WAVE     = 106,
  PARAM_ID_DETAIL_CH1_TXPWR    = 107,
  PARAM_ID_DETAIL_CH1_TXATTEN  = 108,
  PARAM_ID_DETAIL_CH1_ROUTE    = 109,
  PARAM_ID_DETAIL_CH1_ACCESS   = 110,

  /* UART 写参数应答状态 */
  PARAM_ID_UART_ACK_STATE  = 90, /**< 0=idle,1=ok,2=fail,3=pending */
  PARAM_ID_UART_ACK_CMD    = 91, /**< 最近 ACK 的命令字 */

  /* 0x06 统计操作（00开始/01停止/02清零） */
  PARAM_ID_TXRX_OPERATION  = 92,

  /* 设置页 WR 参数（按菜单目录地址） */
  PARAM_ID_DEVICE_NAME_TOKEN = 120,
  PARAM_ID_NET_BIZ_PORT      = 121,
  PARAM_ID_NET_MGMT_IP       = 122,
  PARAM_ID_NET_MGMT_PORT     = 123,
  PARAM_ID_NET_SENSE_IP      = 124,
  PARAM_ID_NET_SENSE_PORT    = 125,
  PARAM_ID_UART_BAUD_IDX     = 126,
  PARAM_ID_UART_DATABITS     = 127,
  PARAM_ID_UART_STOPBITS     = 128,
  PARAM_ID_UART_PARITY       = 129,
  PARAM_ID_UART_FLOW         = 130,
  PARAM_ID_LOC_AUTO_SYNC     = 131,
  PARAM_ID_TIME_AUTO_SYNC    = 132
 };
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* APP_PARAM_DICT_H */
 