import sys

with open("Core/Src/app_menu.c", "r") as f:
    content = f.read()

# Replace 1: OK Handler for FREQ
old_ok = """  if (key == MENU_KEY_OK) {
      int32_t final_val = s_paramInputValue;
      if (final_val > 0) { // Only enforce limit if user actually typed something
          if (final_val < 255) final_val = 255;
          if (final_val > 2500) final_val = 2500;
      }
      if (APP_ParamDict_TrySetValue(PARAM_ID_FIXED_FREQ, final_val)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_FIXED_FREQ);
      }
      return 2;
  }"""
new_ok = """  if (key == MENU_KEY_OK) {
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
content = content.replace(old_ok, new_ok)

# Replace 2: Display for FREQ
old_disp = """      } else {
          (void)snprintf(buf, sizeof(buf), "FREQ:%ld", (long)freq);
      }"""
new_disp = """      } else {
          (void)snprintf(buf, sizeof(buf), "FREQ:%ld", (long)(freq / 1000));
      }"""
content = content.replace(old_disp, new_disp)

with open("Core/Src/app_menu.c", "w") as f:
    f.write(content)

