import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

# Replace the specific nodes
# Remove old function tree completely

text = re.sub(r'static const MenuNode_t s_selfcheck_temp =[\s\S]*?static const MenuNode_t s_function_selfcheck =[\s\S]*?};', '', text)
text = re.sub(r'static const MenuNode_t s_function_member =[\s\S]*?static const MenuNode_t s_function_stats =[\s\S]*?};', '', text, count=1) 
# I might have duplicated some nodes if they were already there, so let's compile and see if there are re-definitions.

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
