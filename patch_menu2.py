import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

handlers_code = """
static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
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
}

static int MenuKeyHandler_TxPower(MenuKey_t key)
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
}
"""

if "MenuKeyHandler_ChannelFreq" not in content:
    idx = content.find("static int MenuKeyHandler_WorkMode(MenuKey_t key)")
    content = content[:idx] + handlers_code + "\n" + content[idx:]
    
content = content.replace(".key_cb    = 0", ".key_cb    = MenuKeyHandler_ChannelFreq", 1)  # Only for s_channel_freq if we replace carefully!

# Wait, this might be unsafe, let's use string replace exactly
old_freq = """static const MenuNode_t s_channel_freq =
{
  .id       = 100,
  .title    = "CH FREQ",
  .parent   = &s_operation_channel,
  .child    = 0,
  .sibling  = &s_channel_txpower,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};"""

new_freq = """static const MenuNode_t s_channel_freq =
{
  .id       = 100,
  .title    = "CH FREQ",
  .parent   = &s_operation_channel,
  .child    = 0,
  .sibling  = &s_channel_txpower,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = MenuKeyHandler_ChannelFreq
};"""

old_pwr = """static const MenuNode_t s_channel_txpower =
{
  .id       = 101,
  .title    = "TX POWER",
  .parent   = &s_operation_channel,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};"""

new_pwr = """static const MenuNode_t s_channel_txpower =
{
  .id       = 101,
  .title    = "TX POWER",
  .parent   = &s_operation_channel,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = MenuKeyHandler_TxPower
};"""

content = content.replace(old_freq, new_freq)
content = content.replace(old_pwr, new_pwr)

# Add function forward declarations
if "static int MenuKeyHandler_ChannelFreq" not in content[:1000]: # top region
    decl = "static int MenuKeyHandler_ChannelFreq(MenuKey_t key);\nstatic int MenuKeyHandler_TxPower(MenuKey_t key);\n"
    idx = content.find("static int MenuKeyHandler_WorkMode(MenuKey_t key);")
    content = content[:idx] + decl + content[idx:]

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

