import re

with open("HMI_Panel_Test_Plan.md", "r", encoding="utf8") as f:
    text = f.read()

menu_tree = """
## 2. 菜单树结构图与功能映射
请在实际测试时，对照下方的固件内已实现的菜单链表结构进行光标跑查，确保所有节点（父/子/兄弟）跳转正常：

```text
[ROOT] MAIN
 ├── (1) OPERATION (操作)
 │    ├── OP MODE       -> 设置/编辑当前工作模式
 │    └── CHANNEL SET   -> 通道设置子菜单
 │         ├── CH FREQ  -> 频点数据展示
 │         └── TX POWER -> 发射功率设置
 │
 ├── (2) FUNCTION (功能与状态)
 │    ├── MEMBERS       -> 节点成员状态(节点数 / 最强RSSI)
 │    ├── TX/RX STATS   -> 数据包收发统计(动态翻滚刷新)
 │    └── SELF CHECK    -> 自检状态子菜单
 │         ├── TEMP     -> 环境温度实时监控
 │         └── FAN      -> 风机启停状态指示
 │
 └── (3) SETTING (系统设置)
      ├── NET           -> 网络 IP 参数配置（预留）
      └── UART          -> 串口本地配置
           └── BAUD     -> 波特率实时展示与修改
```

---
"""

text = re.sub(r'(---\n\n## 2\. 第1阶段)', menu_tree + r'\1', text)

with open("HMI_Panel_Test_Plan.md", "w", encoding="utf8") as f:
    f.write(text)
