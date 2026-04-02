import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# Fix the render: replace `s_bEditMode ? " [*]" : ""` with `s_bEditMode ? " E" : ""` safely everywhere it might occur
content = content.replace('s_bEditMode ? " [*]" : ""', 's_bEditMode ? " E" : ""')
content = content.replace('s_bEditMode ? " [*]" : ""', 's_bEditMode ? " E" : ""')

# Make sure we replace the printlines to use s_paramInputValue when in edit mode
def replace_render(node_id_str, var_name, original_val_get, formatter):
    # This is a bit manual, let's just find and replace the rendering blocks
    pass

import re

# For UART BAUD:
# old: 
#    if (APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &baud)) {
#      char buf[32];
#      (void)snprintf(buf, sizeof(buf), "BAUD:%ld%s", (long)baud, s_bEditMode ? " E" : "");
# new:
#    int32_t val = s_bEditMode ? s_paramInputValue : baud;
#    (void)snprintf(buf, sizeof(buf), "BAUD:%ld%s", (long)val, s_bEditMode ? " E" : "");

content = re.sub(
    r'if \(APP_ParamDict_GetValue\(PARAM_ID_UART_BAUDRATE, &baud\)\)\s*\{\s*char buf\[32\];\s*\(void\)snprintf\(buf, sizeof\(buf\), "BAUD:%ld%s", \(long\)baud, s_bEditMode \? " E" : ""\);\s*BSP_Lcd_PrintLine\(1, buf\);\s*\}',
    r'if (APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &baud)) {\n      char buf[32];\n      int32_t val = s_bEditMode ? s_paramInputValue : baud;\n      (void)snprintf(buf, sizeof(buf), "BAUD:%ld%s", (long)val, s_bEditMode ? " E" : "");\n      BSP_Lcd_PrintLine(1, buf);\n    }',
    content
)

content = re.sub(
    r'if \(APP_ParamDict_GetValue\(PARAM_ID_WORK_MODE, &mode\)\)\s*\{\s*char buf\[32\];\s*\(void\)snprintf\(buf, sizeof\(buf\), "MODE:%ld%s", \(long\)mode, s_bEditMode \? " E" : ""\);\s*BSP_Lcd_PrintLine\(1, buf\);\s*\}',
    r'if (APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &mode)) {\n      char buf[32];\n      int32_t val = s_bEditMode ? s_paramInputValue : mode;\n      (void)snprintf(buf, sizeof(buf), "MODE:%ld%s", (long)val, s_bEditMode ? " E" : "");\n      BSP_Lcd_PrintLine(1, buf);\n    }',
    content
)

content = re.sub(
    r'if \(APP_ParamDict_GetValue\(PARAM_ID_FIXED_FREQ, &freq\)\)\s*\{\s*char fbuf\[16\];\s*char buf\[32\];\s*Menu_FormatMilli\(fbuf, sizeof\(fbuf\), freq\);\s*\(void\)snprintf\(buf, sizeof\(buf\), "FREQ:%s%s", fbuf, s_bEditMode \? " E" : ""\);\s*BSP_Lcd_PrintLine\(1, buf\);\s*\}',
    r'if (APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &freq)) {\n      char fbuf[16];\n      char buf[32];\n      int32_t val = s_bEditMode ? s_paramInputValue : freq;\n      Menu_FormatMilli(fbuf, sizeof(fbuf), val);\n      (void)snprintf(buf, sizeof(buf), "FREQ:%s%s", fbuf, s_bEditMode ? " E" : "");\n      BSP_Lcd_PrintLine(1, buf);\n    }',
    content
)

content = re.sub(
    r'if \(APP_ParamDict_GetValue\(PARAM_ID_TX_POWER, &pwr\)\)\s*\{\s*char buf\[32\];\s*\(void\)snprintf\(buf, sizeof\(buf\), "PWR:%ld%s", \(long\)pwr, s_bEditMode \? " E" : ""\);\s*BSP_Lcd_PrintLine\(1, buf\);\s*\}',
    r'if (APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &pwr)) {\n      char buf[32];\n      int32_t val = s_bEditMode ? s_paramInputValue : pwr;\n      (void)snprintf(buf, sizeof(buf), "PWR:%ld%s", (long)val, s_bEditMode ? " E" : "");\n      BSP_Lcd_PrintLine(1, buf);\n    }',
    content
)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)
