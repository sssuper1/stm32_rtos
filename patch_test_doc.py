import re

with open("HMI_Panel_Test_Plan.md", "r", encoding="utf8") as f:
    text = f.read()

replacement = """## 1. 测试环境与硬件连接
* **主控板**：STM32F103 (或通过串口模块连接开发板)

**管脚分配表 (Pinout) :**
* **OLED 显示屏 (I2C2 接口)**:
  * SCL: `PB10`
  * SDA: `PB11`
  * VCC: 3.3V
  * GND: GND
* **4x4 矩阵键盘**:
  * 行引脚 (输出 / Rows): 
    * R1: `PB5`
    * R2: `PB6`
    * R3: `PB7`
    * R4: `PB8`
  * 列引脚 (输入读取 / Cols):
    * C1: `PB9`
    * C2: `PB12`
    * C3: `PB13`
    * C4: `PB14`
* **USART1 主通信接口 (接 Linux 主板或 USB-TTL)**
  * TX: `PA9`
  * RX: `PA10`
  * GND: 必须与主板/TTL共地
  * 波特率：`115200`，`8N1`
* **心跳指示灯 (可选)**:
  * LED_HEARTBEAT: `PC13`
"""

text = re.sub(r'## 1\. 测试环境与硬件连接[\s\S]*?---\n', replacement + "\n---\n", text)

with open("HMI_Panel_Test_Plan.md", "w", encoding="utf8") as f:
    f.write(text)
