import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

old_hnd = """static int MenuKeyHandler_WorkMode(MenuKey_t key)
{
  int32_t current = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &current))
  {
    return 0;
  }

  if (key == MENU_KEY_UP)
  {
    if (current > 0)
    {
      int32_t next = current - 1;
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, next))
      {
        (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (current < 3)
    {
      int32_t next = current + 1;
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, next))
      {
        (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
    }
    return 1;
  }

  return 0;
}"""

new_hnd = """static int MenuKeyHandler_WorkMode(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      int32_t val = 0;
      APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &val);
      s_paramInputValue = val;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (d <= 3) {
          s_paramInputValue = d;
      }
      return 1;
  }
  if (key == MENU_KEY_UP) {
      if (s_paramInputValue > 0) s_paramInputValue--;
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if (s_paramInputValue < 3) s_paramInputValue++;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      return 2;
  }
  return 1;
}"""

if old_hnd in content:
    content = content.replace(old_hnd, new_hnd)
else:
    print("Could not find old handler exact match, trying regex or manual replace")

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)
