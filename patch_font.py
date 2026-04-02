import sys

with open('Core/Src/bsp_oled.c', 'r') as f:
    content = f.read()

font_add = """
  {'*', {0x14,0x08,0x3E,0x08,0x14,0x00}},
  {'[', {0x00,0x7F,0x41,0x41,0x00,0x00}},
  {']', {0x00,0x41,0x41,0x7F,0x00,0x00}},
"""

if "{'*'," not in content:
    idx = content.find("  {' '")
    content = content[:idx] + font_add[1:] + content[idx:]
    with open('Core/Src/bsp_oled.c', 'w') as f:
        f.write(content)

