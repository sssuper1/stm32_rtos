import sys, re

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# Replace WorkMode Render
render_old = r'''if \(APP_ParamDict_GetValue\(PARAM_ID_WORK_MODE, &mode\)\) \{\s*char buf\[32\];\s*int32_t val = s_bEditMode \? s_paramInputValue : mode;\s*\(void\)snprintf\(buf, sizeof\(buf\), "MODE:%ld%s", \(long\)val, s_bEditMode \? " E" : ""\);\s*BSP_Lcd_PrintLine\(1, buf\);\s*\}'''

render_new = """if (APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &mode)) {
      if (s_bEditMode) {
          BSP_Lcd_PrintLine(1, s_paramInputValue == 0 ? "0  *" : "0");
          BSP_Lcd_PrintLine(2, s_paramInputValue == 1 ? "1  *" : "1");
          BSP_Lcd_PrintLine(3, s_paramInputValue == 2 ? "2  *" : "2");
          BSP_Lcd_PrintLine(4, s_paramInputValue == 3 ? "3  *" : "3");
      } else {
          char buf[32];
          (void)snprintf(buf, sizeof(buf), "MODE:%ld", (long)mode);
          BSP_Lcd_PrintLine(1, buf);
      }
    }"""

content = re.sub(render_old, render_new, content)

# Check MenuKeyHandler_WorkMode
# Right now it has numeric input: 
#   if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
#       int d = Menu_KeyToDigit(key);
#       if (d <= 3) { s_paramInputValue = d; }
#       return 1;
#   }
# This is fine! Let's keep it so they can press '1' directly, or use UP/DOWN. 
# But let's verify if UP/DOWN works.
# Currently Handler has:
#   if (key == MENU_KEY_UP) { if (s_paramInputValue > 0) s_paramInputValue--; return 1; }
#   if (key == MENU_KEY_DOWN) { if (s_paramInputValue < 3) s_paramInputValue++; return 1; }
# This is already perfect for UP/DOWN movement! 
# We just need to make sure the star character is written.

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

