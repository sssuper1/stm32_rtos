with open("Core/Src/app_param_dict.c", "r", encoding="utf8") as f:
    text = f.read()

# Make sure we have the handle declared
if "extern osMutexId_t gParamMutexHandle;" not in text:
    text = text.replace('#include "app_param_dict.h"\n', '#include "app_param_dict.h"\n#include "cmsis_os2.h"\nextern osMutexId_t gParamMutexHandle;\n')

old_func = """bool APP_ParamDict_GetValue(ParamId_t id, int32_t *outVal)
{
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if ((param == NULL) || (outVal == NULL))
  {
    return false;
  }

  *outVal = param->value;
  return true;
}"""

new_func = """bool APP_ParamDict_GetValue(ParamId_t id, int32_t *outVal)
{
  if (gParamMutexHandle != NULL) osMutexAcquire(gParamMutexHandle, osWaitForever);
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if ((param == NULL) || (outVal == NULL))
  {
    if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);
    return false;
  }

  *outVal = param->value;
  if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);
  return true;
}"""

text = text.replace(old_func, new_func)

with open("Core/Src/app_param_dict.c", "w", encoding="utf8") as f:
    f.write(text)

