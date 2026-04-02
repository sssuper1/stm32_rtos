import sys

with open("Core/Src/app_menu.c", "r") as f:
    lines = f.readlines()

def get_line(substr):
    for i, l in enumerate(lines):
        if substr in l:
            return i
    return -1

idx_handlers_start = get_line("static int MenuKeyHandler_ChannelFreq")
idx_handlers_end = get_line("static int MenuKeyHandler_WorkMode(MenuKey_t key);")

idx_nodes_start = get_line("static const MenuNode_t s_operation_mode =")
idx_nodes_end = get_line("/* --- FUNCTION 子菜单 --- */")

idx_workmode_impl = get_line("static int MenuKeyHandler_WorkMode(MenuKey_t key)")
idx_uartbaud_impl = get_line("static int MenuKeyHandler_UartBaud(MenuKey_t key)")


new_handlers = """
// Generic Enum Handler
static int MenuKeyHandler_EnumBase(ParamId_t id, int max_val, MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      int32_t val = 0;
      APP_ParamDict_GetValue(id, &val);
      s_paramInputValue = val;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (d <= max_val) {
          s_paramInputValue = d;
      }
      return 1;
  }
  if (key == MENU_KEY_UP) {
      if (s_paramInputValue > 0) s_paramInputValue--;
      return 1;
  }
  if (key == MENU_KEY_DOWN) {
      if (s_paramInputValue < max_val) s_paramInputValue++;
      return 1;
  }
  if (key == MENU_KEY_OK) {
      if (APP_ParamDict_TrySetValue(id, s_paramInputValue)) {
          (void)APP_ParamUpdate_Request(id);
      }
      s_bEditMode = false;
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      s_bEditMode = false;
      return 2;
  }
  return 1;
}

static int MenuKeyHandler_WorkMode(MenuKey_t key) { return MenuKeyHandler_EnumBase(PARAM_ID_WORK_MODE, 3, key); }
static int MenuKeyHandler_BW(MenuKey_t key)       { return MenuKeyHandler_EnumBase(PARAM_ID_SIGNAL_BANDWIDTH, 3, key); }
static int MenuKeyHandler_MCS(MenuKey_t key)      { return MenuKeyHandler_EnumBase(PARAM_ID_MCS, 7, key); }
static int MenuKeyHandler_Slotlen(MenuKey_t key)  { return MenuKeyHandler_EnumBase(PARAM_ID_SLOTLEN, 3, key); }
static int MenuKeyHandler_HopMode(MenuKey_t key)  { return MenuKeyHandler_EnumBase(PARAM_ID_FREQ_HOP_MODE, 1, key); }

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
      s_bEditMode = false;
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      s_bEditMode = false;
      return 2;
  }
  return 1;
}

static int MenuKeyHandler_FreqGeneric(ParamId_t id, MenuKey_t key)
{
  if (key == MENU_KEY_STAR) {
      s_paramInputValue = 0;
      return 1;
  }
  if (key >= MENU_KEY_NUM_0 && key <= MENU_KEY_NUM_9) {
      int d = Menu_KeyToDigit(key);
      if (s_paramInputValue <= 9999) { 
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
      if (final_val > 0) { 
          if (final_val < 255) final_val = 255;
          if (final_val > 2500) final_val = 2500;
          if (APP_ParamDict_TrySetValue(id, final_val * 1000)) {
              (void)APP_ParamUpdate_Request(id);
          }
      }
      s_bEditMode = false;
      return 2;
  }
  if (key == MENU_KEY_BACK) {
      s_bEditMode = false;
      return 2;
  }
  return 1;
}

static int MenuKeyHandler_ChannelFreq(MenuKey_t key) { return MenuKeyHandler_FreqGeneric(PARAM_ID_FIXED_FREQ, key); }
static int MenuKeyHandler_Freq1(MenuKey_t key) { return MenuKeyHandler_FreqGeneric(PARAM_ID_ADAPT_FREQ1, key); }
static int MenuKeyHandler_Freq2(MenuKey_t key) { return MenuKeyHandler_FreqGeneric(PARAM_ID_ADAPT_FREQ2, key); }
static int MenuKeyHandler_Freq3(MenuKey_t key) { return MenuKeyHandler_FreqGeneric(PARAM_ID_ADAPT_FREQ3, key); }
static int MenuKeyHandler_Freq4(MenuKey_t key) { return MenuKeyHandler_FreqGeneric(PARAM_ID_ADAPT_FREQ4, key); }

"""


render_func = """
extern const MenuNode_t s_operation_mode;

static void FormatOpValStr(const MenuNode_t *n, char *out) {
    if (n->id == MENU_ID_OPERATION_MODE) {
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        sprintf(out, "%d", val);
    } else if (n->id == 100) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &v);
        if (n == s_current && s_bEditMode) {
             if (s_paramInputValue == 0) sprintf(out, "_");
             else sprintf(out, "%ld_", (long)s_paramInputValue);
        } else {
             sprintf(out, "%ld", (long)(v/1000));
        }
    } else if (n->id == 110) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_SIGNAL_BANDWIDTH, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        const char *opts[] = {"2.5", "5", "10", "20"};
        sprintf(out, "%s", (val <= 3) ? opts[val] : "?");
    } else if (n->id == 111) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_MCS, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        sprintf(out, "%d", val);
    } else if (n->id == 112) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_SLOTLEN, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        const char *opts[] = {"0.5", "1", "1.25", "2"};
        sprintf(out, "%s", (val <= 3) ? opts[val] : "?");
    } else if (n->id == 113) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_FREQ_HOP_MODE, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        sprintf(out, "%d", val);
    } else if (n->id == 101) { 
        int32_t v; APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &v);
        int val = (n == s_current && s_bEditMode) ? s_paramInputValue : v;
        sprintf(out, "%d", val);
    } else if (n->id >= 120 && n->id <= 123) {
        ParamId_t p_id = PARAM_ID_ADAPT_FREQ1 + (n->id - 120);
        int32_t v; APP_ParamDict_GetValue(p_id, &v);
        if (n == s_current && s_bEditMode) {
             if (s_paramInputValue == 0) sprintf(out, "_");
             else sprintf(out, "%ld_", (long)s_paramInputValue);
        } else {
             sprintf(out, "%ld", (long)(v/1000));
        }
    }
}

static void Render_OperationList(const MenuNode_t *node)
{
    const MenuNode_t *head = &s_operation_mode;
    const MenuNode_t *curr = head;
    
    int index = 0;
    while (curr && curr != node) {
        index++;
        curr = curr->sibling;
    }
    
    int per_page = 7;
    int page_start = (index / per_page) * per_page;
    
    BSP_Lcd_PrintLine(0, "== OPERATION ==");
    
    curr = head;
    for (int i=0; i<page_start; i++) {
        if (curr) curr = curr->sibling;
    }
    
    for (int row = 1; row <= per_page; row++) {
        if (!curr) {
            BSP_Lcd_PrintLine(row, " ");
            continue;
        }
        
        char buf[32] = {0};
        char val_str[16] = {0};
        
        FormatOpValStr(curr, val_str);
        
        if (curr == node && s_bEditMode) {
           snprintf(buf, sizeof(buf), ">%s [*%s]", curr->title, val_str);
        } else if (curr == node) {
           snprintf(buf, sizeof(buf), ">%s %s", curr->title, val_str);
        } else {
           snprintf(buf, sizeof(buf), " %s %s", curr->title, val_str);
        }
        
        BSP_Lcd_PrintLine(row, buf);
        curr = curr->sibling;
    }
}

"""


new_nodes = """
static const MenuNode_t s_operation_freq4 = { .id = 123, .title = "sel freq4", .parent = &s_operation, .child = 0, .sibling = 0, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_Freq4 };
static const MenuNode_t s_operation_freq3 = { .id = 122, .title = "sel freq3", .parent = &s_operation, .child = 0, .sibling = &s_operation_freq4, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_Freq3 };
static const MenuNode_t s_operation_freq2 = { .id = 121, .title = "sel freq2", .parent = &s_operation, .child = 0, .sibling = &s_operation_freq3, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_Freq2 };
static const MenuNode_t s_operation_freq1 = { .id = 120, .title = "sel freq1", .parent = &s_operation, .child = 0, .sibling = &s_operation_freq2, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_Freq1 };
static const MenuNode_t s_operation_txpower = { .id = 101, .title = "TX POWER", .parent = &s_operation, .child = 0, .sibling = &s_operation_freq1, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_TxPower };
static const MenuNode_t s_operation_hopmode = { .id = 113, .title = "workmode", .parent = &s_operation, .child = 0, .sibling = &s_operation_txpower, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_HopMode };
static const MenuNode_t s_operation_slotlen = { .id = 112, .title = "slotlen", .parent = &s_operation, .child = 0, .sibling = &s_operation_hopmode, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_Slotlen };
static const MenuNode_t s_operation_mcs   = { .id = 111, .title = "MCS", .parent = &s_operation, .child = 0, .sibling = &s_operation_slotlen, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_MCS };
static const MenuNode_t s_operation_bw    = { .id = 110, .title = "BW", .parent = &s_operation, .child = 0, .sibling = &s_operation_mcs, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_BW };
static const MenuNode_t s_operation_freq  = { .id = 100, .title = "CH FREQ", .parent = &s_operation, .child = 0, .sibling = &s_operation_bw, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_ChannelFreq };
const MenuNode_t s_operation_mode = { .id = MENU_ID_OPERATION_MODE, .title = "OP MODE", .parent = &s_operation, .child = 0, .sibling = &s_operation_freq, .render_cb = Render_OperationList, .key_cb = MenuKeyHandler_WorkMode };

"""

# Pre-process forward decls
final_text = "".join(lines)
final_text = final_text.replace("static const MenuNode_t s_operation_channel;\n", "")
final_text = final_text.replace("static const MenuNode_t s_channel_freq;\n", "")
final_text = final_text.replace("static const MenuNode_t s_channel_txpower;\n", "")
final_text = final_text.replace("static int MenuKeyHandler_WorkMode(MenuKey_t key);\n", "")

lines = final_text.splitlines(True) # re-split after replacing

idx_handlers_start = get_line("static int MenuKeyHandler_ChannelFreq")
idx_handlers_end = get_line("static int MenuKeyHandler_UartBaud(MenuKey_t key);")
idx_nodes_start = get_line("static const MenuNode_t s_operation_mode =")
idx_nodes_end = get_line("/* --- FUNCTION 子菜单 --- */")

idx_workmode_impl = get_line("static int MenuKeyHandler_WorkMode(MenuKey_t key)")
idx_uartbaud_impl = get_line("/* 串口波特率编辑逻辑示例：")

# Construct new array
out = lines[:idx_handlers_start] + [new_handlers] + lines[idx_handlers_end:idx_nodes_start] + [render_func] + [new_nodes] + lines[idx_nodes_end:idx_workmode_impl] + lines[idx_uartbaud_impl:]

val = "".join(out)

with open("Core/Src/app_menu.c", "w") as f:
    f.write(val)

