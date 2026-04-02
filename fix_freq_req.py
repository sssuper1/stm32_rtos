import sys, re

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# 1. Update MenuKeyHandler_ChannelFreq to limit 255-2500
old_hnd_freq = """static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
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

new_hnd_freq = """static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      s_paramInputValue = 0;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (s_paramInputValue <= 9999) { // Prevent overflow
          s_paramInputValue = s_paramInputValue * 10 + d;
      }
      return 1;
  }
  if (key == MENU_KEY_HASH) {
      s_paramInputValue /= 10;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      int32_t final_val = s_paramInputValue;
      if (final_val > 0) { // Only enforce limit if user actually typed something
          if (final_val < 255) final_val = 255;
          if (final_val > 2500) final_val = 2500;
      }
      if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, final_val)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
      }
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      return 2;
  }
  return 1;
}"""

content = content.replace(old_hnd_freq, new_hnd_freq)

# 2. Update Render: no decimals, no " E", blank if 0 for s_paramInputValue
# Currently it is:
#     if (APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &freq)) {
#       char fbuf[16];
#       char buf[32];
#       int32_t val = s_bEditMode ? s_paramInputValue : freq;
#       Menu_FormatMilli(fbuf, sizeof(fbuf), val);
#       (void)snprintf(buf, sizeof(buf), "FREQ:%s%s", fbuf, s_bEditMode ? " E" : "");
#       BSP_Lcd_PrintLine(1, buf);
#     }

old_render_freq = """    if (APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &freq)) {
      char fbuf[16];
      char buf[32];
      int32_t val = s_bEditMode ? s_paramInputValue : freq;
      Menu_FormatMilli(fbuf, sizeof(fbuf), val);
      (void)snprintf(buf, sizeof(buf), "FREQ:%s%s", fbuf, s_bEditMode ? " E" : "");
      BSP_Lcd_PrintLine(1, buf);
    }"""

new_render_freq = """    if (APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &freq)) {
      char buf[32];
      if (s_bEditMode) {
          if (s_paramInputValue == 0) {
              (void)snprintf(buf, sizeof(buf), "FREQ: _");
          } else {
              (void)snprintf(buf, sizeof(buf), "FREQ:%ld _", (long)s_paramInputValue);
          }
      } else {
          (void)snprintf(buf, sizeof(buf), "FREQ:%ld", (long)freq);
      }
      BSP_Lcd_PrintLine(1, buf);
    }"""

content = content.replace(old_render_freq, new_render_freq)


with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

