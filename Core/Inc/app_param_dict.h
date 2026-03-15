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
 
 /** 一些示例参数 ID，后续可替换为协议中的正式定义 **/
 enum
 {
   PARAM_ID_WORK_MODE      = 1,  /**< 当前工作模式 */
   PARAM_ID_NET_IP_ADDR    = 2,  /**< 网口 IP 地址（可按分段或索引化存储） */
   PARAM_ID_UART_BAUDRATE  = 3,  /**< 串口波特率   */
   PARAM_ID_ENV_TEMPERATURE = 4  /**< 环境温度 (状态量，只读示例) */
 };
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* APP_PARAM_DICT_H */
 