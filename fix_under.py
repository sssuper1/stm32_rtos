import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

text = re.sub(r'if \(s_paramInputValue == 0\) sprintf\(out, "_"\);\s+else sprintf\(out, "%ld_", \(long\)s_paramInputValue\);',
              'sprintf(out, "%ld", (long)s_paramInputValue);', text)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
