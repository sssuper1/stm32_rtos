import sys

with open("Core/Src/app_menu.c", "r") as f:
    text = f.read()

text = text.replace("extern const MenuNode_t s_operation_mode;", "")
text = text.replace("static void FormatOpValStr(const MenuNode_t *n, char *out) {", "static const MenuNode_t s_operation_mode;\nstatic void FormatOpValStr(const MenuNode_t *n, char *out) {")
text = text.replace("const MenuNode_t s_operation_mode =", "static const MenuNode_t s_operation_mode =")

with open("Core/Src/app_menu.c", "w") as f:
    f.write(text)

