import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

# Modify Render_OperationList to show MAIN->OPERATION
text = re.sub(r'BSP_Lcd_PrintLine\(0, "== OPERATION =="\);',
              'BSP_Lcd_PrintLine(0, "MAIN->OPERATION");', text)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
