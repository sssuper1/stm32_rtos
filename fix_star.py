import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

text = re.sub(r'static int MenuKeyHandler_TxPower\(.*?s_paramInputValue = 0;\s+return 1;\s+\}',
              'static int MenuKeyHandler_TxPower(MenuKey_t key)\n{\n  if (key == MENU_KEY_STAR) {\n      APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &s_paramInputValue);\n      return 1;\n  }', text, flags=re.DOTALL)

text = re.sub(r'static int MenuKeyHandler_FreqGeneric\(.*?s_paramInputValue = 0;\s+return 1;\s+\}',
              'static int MenuKeyHandler_FreqGeneric(ParamId_t id, MenuKey_t key)\n{\n  if (key == MENU_KEY_STAR) {\n      int32_t val=0; APP_ParamDict_GetValue(id, &val);\n      s_paramInputValue = val / 1000;\n      return 1;\n  }', text, flags=re.DOTALL)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
