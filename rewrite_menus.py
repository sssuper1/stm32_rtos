import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

# Make sure we declare the missing prototypes at top
new_decls = """
static void Render_FunctionList(const MenuNode_t *node);
static void Render_SettingList(const MenuNode_t *node);
"""
text = re.sub(r'static void Render_OperationList\(const MenuNode_t \*node\);', 
              r'static void Render_OperationList(const MenuNode_t *node);\n' + new_decls, text)

# Insert the generic List renderers
renderers = """
static void Render_FunctionList(const MenuNode_t *node)
{
    const MenuNode_t *head = node;
    while(head && head->id != 204) {
        head = head->sibling;
    }
    if (!head) head = APP_Menu_GetCurrent();
    while (head && head->id > 204 && head->id <= 206) {
        head = head->parent;
        if(head && head->child) { head = head->child; break; }
    }
    
    // Actually we can just do what Operation did by hardcoding the head node.
    // wait I'll replace this whole block with simpler logic.
}
"""
