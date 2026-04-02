import sys

with open("Core/Src/freertos.c", "r") as f:
    text = f.read()

old_loop = """  uint32_t ignoreKeysUntil = osKernelGetTickCount() + 300U;

  /* Infinite loop */
  for(;;)
  {
    /* 1. Read key input (polling or from buffer) */
    MenuKey_t key = HMI_ReadKey();
    if (osKernelGetTickCount() < ignoreKeysUntil)
    {
      key = MENU_KEY_NONE;
    }

    /* 2. Drive menu state machine if there is a key event */
    if (key != MENU_KEY_NONE)
    {
      (void)APP_Menu_HandleKey(key);
    }

    /* 3. Delay to control polling period (e.g. 10ms) */
    osDelay(10);
  }"""

new_loop = """  uint32_t ignoreKeysUntil = osKernelGetTickCount() + 300U;
  uint32_t lastActionTick = osKernelGetTickCount();

  /* Infinite loop */
  for(;;)
  {
    /* 1. Read key input (polling or from buffer) */
    MenuKey_t key = HMI_ReadKey();
    if (osKernelGetTickCount() < ignoreKeysUntil)
    {
      key = MENU_KEY_NONE;
    }

    /* 2. Drive menu state machine if there is a key event */
    if (key != MENU_KEY_NONE)
    {
      lastActionTick = osKernelGetTickCount();
      (void)APP_Menu_HandleKey(key);
    }
    else
    {
      if ((osKernelGetTickCount() - lastActionTick) > 10000U)
      {
        APP_Menu_ReturnToRoot();
      }
    }

    /* 3. Delay to control polling period (e.g. 10ms) */
    osDelay(10);
  }"""

text = text.replace(old_loop, new_loop)

with open("Core/Src/freertos.c", "w") as f:
    f.write(text)
