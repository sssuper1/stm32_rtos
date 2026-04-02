import sys

with open("Core/Src/app_menu.c", "r") as f:
    content = f.read()

old_init = """const MenuNode_t *APP_Menu_Init(void)
{
  /* 上电即显示 0x08 开机显示参数页。 */
  s_current = &s_function;
  if (s_current->render_cb != 0)
  {
    s_current->render_cb(s_current);
  }
  return s_current;
}"""

new_init = """const MenuNode_t *APP_Menu_Init(void)
{
  s_current = &s_root;
  s_bEditMode = false;
  if (s_current->render_cb != 0)
  {
    s_current->render_cb(s_current);
  }
  return s_current;
}

void APP_Menu_ReturnToRoot(void)
{
  if ((s_current != &s_root) || s_bEditMode)
  {
    s_current = &s_root;
    s_bEditMode = false;
    if (s_current->render_cb != 0)
    {
      s_current->render_cb(s_current);
    }
  }
}"""

content = content.replace(old_init, new_init)

with open("Core/Src/app_menu.c", "w") as f:
    f.write(content)

