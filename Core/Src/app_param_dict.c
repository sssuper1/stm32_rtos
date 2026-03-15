#include "app_param_dict.h"

/**
 * @brief Internal global parameter dictionary.
 *
 * 目前仅填入少量示例参数，证明结构与接口工作正常。
 * 后续可在此表中逐步追加项目文档中的所有参数。
 */
static ParamDef_t s_paramDict[] =
{
  /* 当前工作模式:
   * - 示例取值范围 0~3 (例如：待机/单点/组网/测试)
   * - 1 位整数，无小数
   */
  {
    .id        = PARAM_ID_WORK_MODE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 0
  },

  /* 网口 IP 地址（示例）：
   * 这里先用一个 32 位整型存储 IPv4 地址的整体数值，
   * 以后也可以改成四个独立参数：IP1, IP2, IP3, IP4。
   */
  {
    .id        = PARAM_ID_NET_IP_ADDR,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0x00000000,
    .max_value = 0xFFFFFFFF,
    .scale     = 0,
    .value     = 0xC0A80001  /* 默认 192.168.0.1 */
  },

  /* 串口波特率：
   * 支持的范围 1200 ~ 921600，仅作示例。
   */
  {
    .id        = PARAM_ID_UART_BAUDRATE,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 1200,
    .max_value = 921600,
    .scale     = 0,
    .value     = 115200
  },

  /* 环境温度（只读状态量示例）：
   * 使用 int16 存储，单位 0.1℃，即 251 表示 25.1℃。
   * type 使用 SCALED_1000_I32 也可以，这里示例 int16 语义。
   */
  {
    .id        = PARAM_ID_ENV_TEMPERATURE,
    .type      = PARAM_TYPE_INT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = -400,  /* -40.0 ℃ */
    .max_value =  850,  /*  85.0 ℃ */
    .scale     = 1,
    .value     = 250    /* 25.0 ℃ 示例 */
  }
};

static const uint16_t s_paramCount = (uint16_t)(sizeof(s_paramDict) / sizeof(s_paramDict[0]));

const ParamDef_t *APP_ParamDict_GetTable(uint16_t *outCount)
{
  if (outCount != NULL)
  {
    *outCount = s_paramCount;
  }
  return s_paramDict;
}

ParamDef_t *APP_ParamDict_FindById(ParamId_t id)
{
  for (uint16_t i = 0; i < s_paramCount; ++i)
  {
    if (s_paramDict[i].id == id)
    {
      return &s_paramDict[i];
    }
  }
  return NULL;
}

bool APP_ParamDict_GetValue(ParamId_t id, int32_t *outVal)
{
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if ((param == NULL) || (outVal == NULL))
  {
    return false;
  }

  *outVal = param->value;
  return true;
}

bool APP_ParamDict_TrySetValue(ParamId_t id, int32_t newVal)
{
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if (param == NULL)
  {
    return false;
  }

  /* 只读参数不允许修改 */
  if (param->access != PARAM_ACCESS_RW)
  {
    return false;
  }

  /* 边界检查 */
  if ((newVal < param->min_value) || (newVal > param->max_value))
  {
    return false;
  }

  param->value = newVal;
  return true;
}

