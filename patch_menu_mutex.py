import re

with open("Core/Src/app_menu.c", "r", encoding="utf8") as f:
    text = f.read()

# Add extern osMutexId_t gParamMutexHandle; near the top
if "extern osMutexId_t" not in text:
    text = re.sub(r'(#include "bsp\.h"\n)', r'\1#include "cmsis_os2.h"\nextern osMutexId_t gParamMutexHandle;\n', text)

# Wrap the Render bodies 
def wrap_mutex(func_name, text):
    pattern = r'(static void ' + func_name + r'\(const MenuNode_t \*node\)\n{)([\s\S]*?\n)}'
    def repl(m):
        inner = m.group(2)
        # only lock around the GetValue calls
        new_inner = inner.replace("APP_ParamDict_GetValue", """
    if (gParamMutexHandle != NULL) osMutexAcquire(gParamMutexHandle, osWaitForever);
    bool _tmp = APP_ParamDict_GetValue""").replace("&val)) {", "&val));\n    if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);\n    if (_tmp) {")\
        .replace("&cnt)) {", "&cnt));\n    if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);\n    if (_tmp) {")\
        .replace("&rssi)) {", "&rssi));\n    if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);\n    if (_tmp) {")\
        .replace("APP_ParamDict_GetValue(PARAM_ID_ETH_TX_CNT, &tx);", "if (gParamMutexHandle != NULL) osMutexAcquire(gParamMutexHandle, osWaitForever);\n  APP_ParamDict_GetValue(PARAM_ID_ETH_TX_CNT, &tx);")\
        .replace("APP_ParamDict_GetValue(PARAM_ID_ETH_RX_CNT, &rx);", "APP_ParamDict_GetValue(PARAM_ID_ETH_RX_CNT, &rx);\n  if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);")
        return m.group(1) + new_inner + "}"
    return re.sub(pattern, repl, text)

text = wrap_mutex("Render_SelfCheck", text)
text = wrap_mutex("Render_Member", text)
text = wrap_mutex("Render_Stats", text)

with open("Core/Src/app_menu.c", "w", encoding="utf8") as f:
    f.write(text)

