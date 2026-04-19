# FW_CommLab_01_I2C_Loopback_IT
## 🛠 Hardware Setup

Board: **NUCLEO-H723ZG**

### Wiring

| Signal         | Connection        |
| -------------- | ----------------- |
| I2C1_SCL (PB8) | → I2C2_SCL (PB10) |
| I2C1_SDA (PB9) | → I2C2_SDA (PB11) |
| GND            | → GND             |

---

### ⚠️ Pull-up Configuration

This test is currently performed **without external pull-up resistors**
(using only internal pull-up or short-distance wiring).

> This setup may work in short-distance, low-speed conditions,
> but **does NOT comply with I2C electrical specifications**.

---

### ✅ Recommended Configuration (Standard I2C)

External pull-up resistors should be added:

* SDA → 3.3V (4.7kΩ)
* SCL → 3.3V (4.7kΩ)

This ensures:

* Proper signal rising edges
* Stable communication
* Compliance with I2C standard

---
### ❌ FAIL Condition
A FAIL may occur when:
- I2C communication is not properly initialized
- Data is not received correctly
- Data mismatch occurs
- Bus error / timeout happens
![I2C Loopback Test FAIL](docs/images/Fail.png)
---

### ✅ PASS Condition
A PASS indicates:
- `HELLO_I2C` is received correctly
- Returned data matches transmitted data
- Communication is stable
![I2C Loopback Test FAIL](docs/images/Pass.png)

### 🧠 Note

Internal pull-up resistors are typically too weak (~30kΩ–50kΩ)
and may cause unreliable communication in real applications.
