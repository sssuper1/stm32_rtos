import sys

with open("Core/Src/app_menu.c", "r") as f:
    content = f.read()

# I messed up s_bEditMode scope.
content = content.replace("static bool s_bEditMode = false;", "")

decl = """/* -------------------- 狀態機內部變量 -------------------- */

// Move globals to top scope!
static bool s_bEditMode = false;

static const MenuNode_t *s_current;
"""
content = decl + content.replace("static const MenuNode_t *s_current = &s_root;", "")

with open("Core/Src/app_menu.c", "w") as f:
    f.write(content)
