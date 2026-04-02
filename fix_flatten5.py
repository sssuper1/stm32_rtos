import re

with open('Core/Src/app_menu.c', 'r') as f:
    text = f.read()

flattened_func = """
static const MenuNode_t s_selfcheck_fan =
{
  .id       = 201,
  .title    = "FAN",
  .parent   = &s_function,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_FunctionList,
  .key_cb    = 0
};

static const MenuNode_t s_selfcheck_temp =
{
  .id       = 200,
  .title    = "TEMP",
  .parent   = &s_function,
  .child    = 0,
  .sibling  = &s_selfcheck_fan,
  .render_cb = Render_FunctionList,
  .key_cb    = 0
};

static const MenuNode_t s_function_stats =
{
  .id       = 203,
  .title    = "TX/RX STATS",
  .parent   = &s_function,
  .child    = 0,
  .sibling  = &s_selfcheck_temp,
  .render_cb = Render_FunctionList,
  .key_cb    = 0
};

static const MenuNode_t s_function_member =
{
  .id       = 204,
  .title    = "MEMBERS",
  .parent   = &s_function,
  .child    = 0,
  .sibling  = &s_function_stats,
  .render_cb = Render_FunctionList,
  .key_cb    = 0
};
"""

text = re.sub(r'(static const MenuNode_t s_function =)', flattened_func + r'\n\1', text)

with open('Core/Src/app_menu.c', 'w') as f:
    f.write(text)
