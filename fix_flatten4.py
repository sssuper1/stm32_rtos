import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

# Replace the specific nodes
# s_function and s_setting will point to their new flat children

text = re.sub(r'static const MenuNode_t s_setting_net =[\s\S]*?static const MenuNode_t s_setting_uart_baud =[\s\S]*?};', '', text)

flattened_nodes = """

// SETTING

static const MenuNode_t s_setting_uart_baud =
{
  .id       = 302,
  .title    = "UART BAUD",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_SettingList,
  .key_cb    = MenuKeyHandler_UartBaud
};

static const MenuNode_t s_setting_net =
{
  .id       = 301,
  .title    = "NET IP",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = &s_setting_uart_baud,
  .render_cb = Render_SettingList,
  .key_cb    = 0
};
"""

text = re.sub(r'(static const MenuNode_t s_setting =)', flattened_nodes + r'\n\1', text)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
