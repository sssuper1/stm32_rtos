import sys

with open("Core/Src/app_menu.c", "r") as f:
    content = f.read()

# Replace WorkMode OK Handler
old_workmode = """  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
      return 2;
  }"""
new_workmode = """  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
      s_bEditMode = false;
      return 2;
  }"""
content = content.replace(old_workmode, new_workmode)

# Replace TxPower OK Handler
old_txpower = """  if (key == MENU_KEY_OK) {
      if (s_paramInputValue > 33) s_paramInputValue = 33;
      if (APP_ParamDict_TrySetValue(PARAM_ID_TX_POWER, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_TX_POWER);
      }
      return 2;
  }"""
new_txpower = """  if (key == MENU_KEY_OK) {
      if (s_paramInputValue > 33) s_paramInputValue = 33;
      if (APP_ParamDict_TrySetValue(PARAM_ID_TX_POWER, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_TX_POWER);
      }
      s_bEditMode = false;
      return 2;
  }"""
content = content.replace(old_txpower, new_txpower)

# Replace CH FREQ OK Handler
old_freq = """  if (key == MENU_KEY_OK) {
      int32_t final_val = s_paramInputValue;
      if (final_val > 0) { 
          if (final_val < 255) final_val = 255;
          if (final_val > 2500) final_val = 2500;
          if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, final_val * 1000)) {
              (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
          }
      }
      return 2;
  }"""
new_freq = """  if (key == MENU_KEY_OK) {
      int32_t final_val = s_paramInputValue;
      if (final_val > 0) { 
          if (final_val < 255) final_val = 255;
          if (final_val > 2500) final_val = 2500;
          if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, final_val * 1000)) {
              (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
          }
      }
      s_bEditMode = false;
      return 2;
  }"""
content = content.replace(old_freq, new_freq)


with open("Core/Src/app_menu.c", "w") as f:
    f.write(content)
