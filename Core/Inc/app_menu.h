/**
 * @file app_menu.h
 * @brief Multi-level menu state machine interface.
 *
 * 本模块提供一个通用的菜单树结构和导航接口：
 * - 菜单以“节点”形式组织成树（父 / 子 / 兄弟 指针）
 * - 通过统一的按键事件（上 / 下 / 确认 / 返回）在树上移动
 * - 每个节点可关联渲染回调与按键处理回调，后续与 LCD/UI 解耦
 *
 * 目前仅实现少量示例菜单项，验证状态机工作正常。
 * 后续可以在不改接口的前提下，扩展为完整的 5 级菜单。
 */

#ifndef APP_MENU_H
#define APP_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct MenuNode MenuNode_t;

/**
 * @brief 菜单键值类型（抽象层，不直接绑定具体 GPIO）。
 */
typedef enum
{
  MENU_KEY_NONE = 0,
  MENU_KEY_UP,
  MENU_KEY_DOWN,
  MENU_KEY_OK,
  MENU_KEY_BACK
} MenuKey_t;

/**
 * @brief 菜单节点渲染回调函数类型。
 *
 * UI 层在需要刷新屏幕时调用此回调，根据当前选中节点进行显示。
 */
typedef void (*MenuRenderCallback_t)(const MenuNode_t *node);

/**
 * @brief 菜单节点在“编辑/叶子节点”场景下的专用按键处理回调。
 *
 * 若返回非 0，表示回调已完全处理此次按键（例如在数值编辑模式），
 * 状态机将不再执行默认导航行为。
 */
typedef int (*MenuKeyHandler_t)(MenuKey_t key);

/**
 * @brief 菜单节点定义。
 */
struct MenuNode
{
  uint16_t             id;          /**< 节点 ID（可与业务逻辑绑定） */
  const char          *title;       /**< 显示名称（用于 UI 文本）   */
  const MenuNode_t    *parent;      /**< 父节点                     */
  const MenuNode_t    *child;       /**< 第一个子节点               */
  const MenuNode_t    *sibling;     /**< 同级下一个节点             */
  MenuRenderCallback_t render_cb;   /**< 渲染回调                   */
  MenuKeyHandler_t     key_cb;      /**< 叶子节点专用按键处理回调   */
};

/**
 * @brief 初始化菜单状态机，返回根节点指针。
 *
 * 应在系统启动时调用一次（例如 HMI_Task 初始化阶段）。
 */
const MenuNode_t *APP_Menu_Init(void);

/**
 * @brief 获取当前选中的菜单节点。
 */
const MenuNode_t *APP_Menu_GetCurrent(void);

/**
 * @brief 处理一次按键事件，内部完成节点跳转。
 *
 * - 优先调用当前节点的 key_cb（若存在），可用于参数编辑模式。
 * - 若 key_cb 返回 0 或不存在，则执行默认导航（上下/确认/返回）。
 *
 * @param key   本次按键
 * @return      处理后当前节点指针（便于调用方立刻刷新 UI）
 */
const MenuNode_t *APP_Menu_HandleKey(MenuKey_t key);

/**
 * @brief 一个简单的默认渲染回调示例（仅输出标题占位）。
 *
 * 正式接入 LCD 时，可在 UI 层自定义更复杂的渲染函数，并挂到节点上。
 */
void APP_Menu_DefaultRender(const MenuNode_t *node);

/* 一些示例节点 ID，后续可扩展为完整菜单树 */
enum
{
  MENU_ID_ROOT = 0,
  MENU_ID_OPERATION,
  MENU_ID_FUNCTION,
  MENU_ID_SETTING,

  MENU_ID_OPERATION_MODE,     /* 操作 -> 操作模式设置 */
  MENU_ID_OPERATION_PARAM,    /* 操作 -> 参数设置     */

  MENU_ID_SETTING_NET,        /* 设置 -> 网口设置     */
  MENU_ID_SETTING_UART,       /* 设置 -> 串口设置     */

  /* 示例：设置 -> 串口设置 -> 波特率 */
  MENU_ID_SETTING_UART_BAUD
};

#ifdef __cplusplus
}
#endif

#endif /* APP_MENU_H */

