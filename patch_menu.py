import sys

with open('Core/Src/app_menu.c', 'r') as f:
    lines = f.read().split('\n')

out = []
i = 0
found_s_current = False
while i < len(lines):
    line = lines[i]
    if "static const MenuNode_t *s_current = &s_root;" in line:
        out.append(line)
        out.append("static bool s_bEditMode = false;")
        found_s_current = True
        i += 1
        continue
    
    if "const MenuNode_t *APP_Menu_HandleKey(MenuKey_t key)" in line:
        # replace the whole function
        out.append(line)
        out.append("{")
        out.append("  if (key == MENU_KEY_NONE)")
        out.append("  {")
        out.append("    return s_current;")
        out.append("  }")
        out.append("")
        out.append("  if ((s_current != 0) && (s_current->key_cb != 0))")
        out.append("  {")
        out.append("    if (s_bEditMode)")
        out.append("    {")
        out.append("      if ((key == MENU_KEY_OK) || (key == MENU_KEY_BACK))")
        out.append("      {")
        out.append("        s_bEditMode = false;")
        out.append("        if (s_current->render_cb != 0) s_current->render_cb(s_current);")
        out.append("        return s_current;")
        out.append("      }")
        out.append("      (void)s_current->key_cb(key);")
        out.append("      if (s_current->render_cb != 0) s_current->render_cb(s_current);")
        out.append("      return s_current;")
        out.append("    }")
        out.append("    else")
        out.append("    {")
        out.append("      if (key == MENU_KEY_STAR)")
        out.append("      {")
        out.append("        s_bEditMode = true;")
        out.append("        if (s_current->render_cb != 0) s_current->render_cb(s_current);")
        out.append("        return s_current;")
        out.append("      }")
        out.append("    }")
        out.append("  }")
        out.append("  else")
        out.append("  {")
        out.append("    s_bEditMode = false;")
        out.append("  }")
        out.append("")
        out.append("  if (!s_bEditMode)")
        out.append("  {")
        out.append("    menu_default_navigate(key);")
        out.append("    s_bEditMode = false;")
        out.append("  }")
        out.append("  return s_current;")
        out.append("}")
        
        # skip existing function lines
        i += 1
        while i < len(lines):
            if lines[i].startswith("}") and "return s_current;" in lines[i-1]:
                i += 1
                break
            i += 1
        continue
    
    if "if (node->id == MENU_ID_SETTING_UART_BAUD)" in line:
        # replace the render logic
        out.append(line)
        out.append("  {")
        out.append("    int32_t baud = 0;")
        out.append("    if (APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &baud))")
        out.append("    {")
        out.append("      char buf[32];")
        out.append("      (void)snprintf(buf, sizeof(buf), \"BAUD:%ld%s\", (long)baud, s_bEditMode ? \" [*]\" : \"\");")
        out.append("      BSP_Lcd_PrintLine(1, buf);")
        out.append("    }")
        out.append("    return;")
        out.append("  }")
        
        i += 1
        while i < len(lines):
            if lines[i] == "  }":
                i += 1
                break
            i += 1
        continue
    
    if "if (node->id == MENU_ID_OPERATION_MODE)" in line:
        out.append(line)
        out.append("  {")
        out.append("    int32_t mode = 0;")
        out.append("    if (APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &mode))")
        out.append("    {")
        out.append("      char buf[32];")
        out.append("      (void)snprintf(buf, sizeof(buf), \"MODE:%ld%s\", (long)mode, s_bEditMode ? \" [*]\" : \"\");")
        out.append("      BSP_Lcd_PrintLine(1, buf);")
        out.append("    }")
        out.append("    return;")
        out.append("  }")
        out.append("")
        out.append("  if (node->id == 100) /* CH FREQ */")
        out.append("  {")
        out.append("    int32_t freq = 0;")
        out.append("    if (APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &freq))")
        out.append("    {")
        out.append("      char fbuf[16];")
        out.append("      char buf[32];")
        out.append("      Menu_FormatMilli(fbuf, sizeof(fbuf), freq);")
        out.append("      (void)snprintf(buf, sizeof(buf), \"FREQ:%s%s\", fbuf, s_bEditMode ? \" [*]\" : \"\");")
        out.append("      BSP_Lcd_PrintLine(1, buf);")
        out.append("    }")
        out.append("    return;")
        out.append("  }")
        out.append("")
        out.append("  if (node->id == 101) /* TX POWER */")
        out.append("  {")
        out.append("    int32_t pwr = 0;")
        out.append("    if (APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &pwr))")
        out.append("    {")
        out.append("      char buf[32];")
        out.append("      (void)snprintf(buf, sizeof(buf), \"PWR:%ld%s\", (long)pwr, s_bEditMode ? \" [*]\" : \"\");")
        out.append("      BSP_Lcd_PrintLine(1, buf);")
        out.append("    }")
        out.append("    return;")
        out.append("  }")
        
        i += 1
        while i < len(lines):
            if lines[i] == "  }":
                i += 1
                break
            i += 1
        continue
        
    out.append(line)
    i += 1

with open('Core/Src/app_menu.c', 'w') as f:
    f.write('\n'.join(out))

