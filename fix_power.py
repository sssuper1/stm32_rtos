import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

old_hnd = """static int MenuKeyHandler_TxPower(MenuKey_t key)
{
  int32_t current = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &current)) return 0;
  
  if (key == MENU_KEY_UP) {
      if (current < 33) {
          int32_t next = current + 1;
          if (APP_ParamDict_TrySetValue(PARAM_ID_TX_POWER, next)) {
              (void)APP_ParamUpdate_Request(PARAM_ID_TX_POWER);
          }
      }
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if (current > 0) {
          int32_t next = current - 1;
          if (APP_ParamDict_TrySetValue(PARAM_ID_TX_POWER, next)) {
              (void)APP_ParamUpdate_Request(PARAM_ID_TX_POWER);
          }
      }
      return 1;
  }
  return 0;
}"""

new_hnd = """static int MenuKeyHandler_TxPower(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      s_paramInputValue = 0;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (s_paramInputValue < 33) {
          s_paramInputValue = s_paramInputValue * 10 + d;
      }
      return 1;
  }
  if (key == MENU_KEY_HASH) {
      s_paramInputValue /= 10;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (s_paramInputValue > 33) s_paramInputValue = 33;
      if (APP_ParamDict_TrySetValue(PARAM_ID_TX_POWER, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_TX_POWER);
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

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

