import sys

with open("Core/Src/app_menu.c", "r") as f:
    content = f.read()

# I put globals before includes. Big mistake.
content = content.replace("""/* -------------------- 狀態機內部變量 -------------------- */\n\n// Move globals to top scope!\nstatic bool s_bEditMode = false;\n\nstatic const MenuNode_t *s_current;\n""", "")

decl2 = """
/* -------------------- 狀態機內部變量 -------------------- */
static bool s_bEditMode = false;
static const MenuNode_t *s_current;
"""

content = content.replace("static int32_t s_paramInputValue = 0;", decl2 + "\nstatic int32_t s_paramInputValue = 0;")

with open("Core/Src/app_menu.c", "w") as f:
    f.write(content)
