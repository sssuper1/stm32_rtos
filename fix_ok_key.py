import sys

with open('Core/Src/app_menu.c', 'r') as f:
    content = f.read()

# I see what happened. In the very first patch we injected `if ((key == MENU_KEY_OK) || (key == MENU_KEY_BACK)) { s_bEditMode = false; ... return s_current; }`
# Which intercepts the OK key BEFORE it ever reaches `s_current->key_cb(key)` !!!
# So the handler never receives OK!

old_str = """    if (s_bEditMode)
    {
      if ((key == MENU_KEY_OK) || (key == MENU_KEY_BACK))
      {
        s_bEditMode = false;
        if (s_current->render_cb != 0) s_current->render_cb(s_current);
        return s_current;
      }
      (void)s_current->key_cb(key);
      if (s_current->render_cb != 0) s_current->render_cb(s_current);
      return s_current;
    }"""

new_str = """    if (s_bEditMode)
    {
      int ret = s_current->key_cb(key);
      if (ret == 2)
      {
        s_bEditMode = false;
      }
      if (s_current->render_cb != 0) s_current->render_cb(s_current);
      return s_current;
    }"""

if old_str in content:
    content = content.replace(old_str, new_str)
else:
    print("WARNING: Could not find old_str in HandleKey")

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(content)

