import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# Fix the render: replace [*] with E
content = content.replace("s_bEditMode ? \" [*]\" : \"\"", "s_bEditMode ? \" E\" : \"\"")

# Fix the handle key loop
handle_key_src_old = """const MenuNode_t *APP_Menu_HandleKey(MenuKey_t key)
{
  if (key == MENU_KEY_NONE)
  {
    return s_current;
  }

  if ((s_current != 0) && (s_current->key_cb != 0))
  {
    if (s_bEditMode)
    {
      if ((key == MENU_KEY_OK) || (key == MENU_KEY_BACK))
      {
        s_bEditMode = false;
        if (s_current->render_cb != 0) s_current->render_cb(s_current);
        return s_current;
      }
      (void)s_current->key_cb(key);
      if (s_current->render_cb != 0) s_current->render_cb(s_current);
      return s_current;
    }
    else
    {
      if (key == MENU_KEY_STAR)
      {
        s_bEditMode = true;
        if (s_current->render_cb != 0) s_current->render_cb(s_current);
        return s_current;
      }
    }
  }
  else
  {
    s_bEditMode = false;
  }

  if (!s_bEditMode)
  {
    menu_default_navigate(key);
    s_bEditMode = false;
  }
  return s_current;
}"""

handle_key_src_new = """const MenuNode_t *APP_Menu_HandleKey(MenuKey_t key)
{
  if (key == MENU_KEY_NONE)
  {
    return s_current;
  }

  if ((s_current != 0) && (s_current->key_cb != 0))
  {
    if (s_bEditMode)
    {
      int ret = s_current->key_cb(key);
      if (ret == 2)
      {
        s_bEditMode = false;
      }
      if (s_current->render_cb != 0) s_current->render_cb(s_current);
      return s_current;
    }
    else
    {
      if (key == MENU_KEY_STAR)
      {
        s_bEditMode = true;
        (void)s_current->key_cb(key); // pass STAR to init buffer
        if (s_current->render_cb != 0) s_current->render_cb(s_current);
        return s_current;
      }
    }
  }
  else
  {
    s_bEditMode = false;
  }

  if (!s_bEditMode)
  {
    menu_default_navigate(key);
    s_bEditMode = false;
  }
  return s_current;
}"""

content = content.replace(handle_key_src_old, handle_key_src_new)

# Generate new handlers. We will replace the whole file from MenuKeyHandler_WorkMode onwards to fix the handlers.
idx = content.find("static int MenuKeyHandler_ChannelFreq(MenuKey_t key)")
idx_tx = content.find("static int MenuKeyHandler_TxPower(MenuKey_t key)")

handlers_new = """
static int MenuKeyHandler_ChannelFreq(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      s_paramInputValue = 0;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (s_paramInputValue <= 999999) { // arbitrary limit to prevent overflow
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
}

static int MenuKeyHandler_TxPower(MenuKey_t key)
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
}

static int MenuKeyHandler_WorkMode(MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      int32_t val = 0;
      APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &val);
      s_paramInputValue = val;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (d <= 3) { // Mode usually 0-3
          s_paramInputValue = d;
      }
      return 1;
  }
  if (key == MENU_KEY_UP) {
      if (s_paramInputValue > 0) s_paramInputValue--;
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if (s_paramInputValue < 3) s_paramInputValue++;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      return 2;
  }
  return 1;
}

static int MenuKeyHandler_UartBaud(MenuKey_t key)
{
  static const int32_t baudList[] = {9600, 19200, 38400, 57600, 115200, 230400};
  const uint32_t baudCount = (uint32_t)(sizeof(baudList) / sizeof(baudList[0]));
  
  if (key == MENU_KEY_STAR) {
      int32_t val = 0;
      APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &val);
      s_paramInputValue = val;
      return 1;
  }
  
  uint32_t idx = 0;
  for (uint32_t i=0; i<baudCount; i++) {
      if (baudList[i] == s_paramInputValue) {
          idx = i; break;
      }
  }

  if (key == MENU_KEY_UP) {
      if (idx > 0) {
          s_paramInputValue = baudList[idx - 1];
      }
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if ((idx + 1) < baudCount) {
          s_paramInputValue = baudList[idx + 1];
      }
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(PARAM_ID_UART_BAUDRATE, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(PARAM_ID_UART_BAUDRATE);
      }
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      return 2;
  }
  return 1;
}
"""

# Find where to replace handlers
start_handlers = content.find("static int MenuKeyHandler_ChannelFreq(MenuKey_t key)")
if start_handlers != -1:
    end_handlers = content.find("static int MenuKeyHandler_OperationParam(MenuKey_t key)")
    if end_handlers == -1:
        end_handlers = start_handlers # should not happen
        print("error finding end of handlers")
    else:
        content = content[:start_handlers] + handlers_new + content[end_handlers:]

# Wait, if I'm using `s_paramInputValue` for rendering when in edit mode, I need to update the DEFAULT RENDER func.
# Right now, `BSP_Lcd_PrintLine(1, buf)` prints the dictionary value!
# Let's fix the default render function to use s_paramInputValue if in s_bEditMode.
