import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

# First replace Operation header
text = re.sub(r'BSP_Lcd_PrintLine\(0, "== OPERATION =="\);',
              'BSP_Lcd_PrintLine(0, "MAIN-->OPERATION");', text)

# Insert externs for the new head nodes
head_decls = """
extern const MenuNode_t s_function_member;
extern const MenuNode_t s_setting_net;
"""
text = re.sub(r'extern const MenuNode_t s_operation_mode;', head_decls + '\nextern const MenuNode_t s_operation_mode;', text)


# Generate the list renderers right after Render_OperationList
func_renderers = """
static void Render_FunctionList(const MenuNode_t *node)
{
    const MenuNode_t *head = &s_function_member;
    const MenuNode_t *curr = head;
    
    int index = 0;
    while (curr && curr != node) {
        index++;
        curr = curr->sibling;
    }
    
    int per_page = 7;
    int page_start = (index / per_page) * per_page;
    
    BSP_Lcd_PrintLine(0, "MAIN-->FUNCTION");
    
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
        
        // Render specific value logic
        if (curr->id == 204) { sprintf(val_str, "%d", 0); } // MEMBERS
        else if (curr->id == 203) { sprintf(val_str, "%d/%d", 0, 0); } // TX/RX
        else if (curr->id == 200) { sprintf(val_str, "%dC", 35); } // TEMP
        else if (curr->id == 201) { sprintf(val_str, "OFF"); } // FAN
        
        snprintf(buf, sizeof(buf), "%c%s %s", (curr == node) ? '>' : ' ', curr->title, val_str);
        
        BSP_Lcd_PrintLine(row, buf);
        curr = curr->sibling;
    }
}

static void Render_SettingList(const MenuNode_t *node)
{
    const MenuNode_t *head = &s_setting_net;
    const MenuNode_t *curr = head;
    
    int index = 0;
    while (curr && curr != node) {
        index++;
        curr = curr->sibling;
    }
    
    int per_page = 7;
    int page_start = (index / per_page) * per_page;
    
    BSP_Lcd_PrintLine(0, "MAIN-->SETTING");
    
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
        
        if (curr->id == 301) { sprintf(val_str, "IP..."); }
        else if (curr->id == 302) {
             int32_t v; APP_ParamDict_GetValue(PARAM_ID_UART_BAUD, &v);
             int val = (curr == node && s_bEditMode) ? s_paramInputValue : v;
             sprintf(val_str, "%d", val);
        }
        
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
text = re.sub(r'(static const MenuNode_t s_operation_freq4 = )', func_renderers + r'\1', text)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
