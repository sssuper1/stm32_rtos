import sys

with open("Core/Inc/app_menu.h", "r") as f:
    content = f.read()

content = content.replace("void APP_Menu_RefreshCurrent(void);", "void APP_Menu_RefreshCurrent(void);\n\nvoid APP_Menu_ReturnToRoot(void);")

with open("Core/Inc/app_menu.h", "w") as f:
    f.write(content)
