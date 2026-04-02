import sys

with open("Core/Inc/app_param_dict.h", "r") as f:
    text = f.read()

text = text.replace("PARAM_ID_TX_POWER_ATTEN  = 75,", "PARAM_ID_TX_POWER_ATTEN  = 75,\n  PARAM_ID_MCS             = 76,\n  PARAM_ID_SLOTLEN         = 77,")

with open("Core/Inc/app_param_dict.h", "w") as f:
    f.write(text)

with open("Core/Src/app_param_dict.c", "r") as f:
    text = f.read()

new_params = """  {
    .id        = PARAM_ID_MCS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 7,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SLOTLEN,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 3
  },
"""

text = text.replace("/* 0x09 邻居信息 */", new_params + "  /* 0x09 邻居信息 */")

with open("Core/Src/app_param_dict.c", "w") as f:
    f.write(text)

