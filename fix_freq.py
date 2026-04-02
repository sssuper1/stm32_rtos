import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# Make sure we didn't patch MenuKeyHandler_ChannelFreq poorly before
old_hnd = """static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
{
  int32_t current = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &current)) return 0;
  
  if (key == MENU_KEY_UP) {
      int32_t next = current + 1000;
      if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, next)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
      }
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if (current >= 1000) {
          int32_t next = current - 1000;
          if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, next)) {
              (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
          }
      }
      return 1;
  }
  return 0;
}"""

new_hnd = """static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      s_paramInputValue = 0;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (s_paramInputValue <= 999999) { // Prevent overflow
          s_paramInputValue = s_paramInputValue * 10 + d;
      }
      return 1;
  }
  if (key == MENU_KEY_HASH) {
      s_paramInputValue /= 10;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
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
    print("Could not find old freq block")

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

