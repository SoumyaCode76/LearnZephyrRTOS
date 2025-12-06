# Zephyr RTOS Devicetree Overlay Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Overlay File Basics](#overlay-file-basics)
3. [Modifying Existing Properties](#modifying-existing-properties)
4. [Adding New Properties](#adding-new-properties)
5. [Deleting Properties](#deleting-properties)
6. [Creating New Nodes](#creating-new-nodes)
7. [Working with Aliases and Chosen](#working-with-aliases-and-chosen)
8. [Common Property Types](#common-property-types)
9. [Practical Examples](#practical-examples)
10. [Best Practices](#best-practices)

---

## Introduction

**Devicetree overlays** (`app.overlay`) allow you to modify or extend the board's default devicetree without editing the board definition files. They use the same `.dts` syntax as devicetree files.

### When to Use Overlays
- Modify peripheral configurations (UART baud rate, I2C speed, etc.)
- Enable/disable peripherals
- Add custom hardware definitions (LEDs, buttons, sensors)
- Override pin assignments
- Set application-specific chosen nodes

---

## Overlay File Basics

### File Location
Place your overlay file in your application directory:
```
my_app/
├── CMakeLists.txt
├── prj.conf
├── app.overlay          ← Your overlay file
└── src/
    └── main.c
```

### Basic Syntax
```dts
// Reference and modify existing node
&existing_node_label {
    property = <value>;
};

// Create new nodes
/ {
    new_node {
        property = <value>;
    };
};
```

---

## Modifying Existing Properties

### Syntax: Reference Node with `&label`
Use `&` followed by the node label to reference an existing devicetree node.

### Example 1: Change UART Baud Rate
```dts
&uart0 {
    current-speed = <115200>;  // Modify existing property
};
```

### Example 2: Enable Disabled Peripheral
```dts
&i2c1 {
    status = "okay";  // Change from "disabled" to "okay"
};
```

### Example 3: Change GPIO Configuration
```dts
&gpioa {
    gpio-reserved-ranges = <0 2>, <15 1>;  // Reserve pins PA0, PA1, PA15
};
```

### Example 4: Modify Multiple Properties
```dts
&spi2 {
    status = "okay";
    clock-frequency = <8000000>;
    cs-gpios = <&gpiob 12 GPIO_ACTIVE_LOW>;
};
```

---

## Adding New Properties

You can add properties that don't exist in the base devicetree.

### Example 1: Add Custom Property
```dts
&uart2 {
    my-custom-property = <42>;
    my-string-property = "custom_value";
};
```

### Example 2: Add Pin Control Configuration
```dts
&usart1 {
    pinctrl-0 = <&usart1_tx_pa9 &usart1_rx_pa10>;
    pinctrl-1 = <&usart1_tx_sleep_pa9 &usart1_rx_sleep_pa10>;
    pinctrl-names = "default", "sleep";
};
```

### Example 3: Add DMA Configuration
```dts
&spi1 {
    dmas = <&dma1 3 0x20440 0x0>,
           <&dma1 2 0x20480 0x0>;
    dma-names = "tx", "rx";
};
```

---

## Deleting Properties

### Syntax: Use `/delete-property/`
```dts
&node_label {
    /delete-property/ property-name;
};
```

### Example 1: Remove Interrupt Configuration
```dts
&gpio0 {
    /delete-property/ interrupts;
};
```

### Example 2: Remove Pin Assignment
```dts
&uart3 {
    /delete-property/ pinctrl-0;
    /delete-property/ pinctrl-names;
};
```

---

## Creating New Nodes

### Syntax: Add Under Root `/` or Existing Node
```dts
/ {
    new_node: node_label {
        compatible = "vendor,device";
        property = <value>;
    };
};
```

### Example 1: Add Custom LED
```dts
/ {
    leds {
        compatible = "gpio-leds";
        led0: led_0 {
            gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
            label = "Green LED";
        };
        led1: led_1 {
            gpios = <&gpiob 14 GPIO_ACTIVE_HIGH>;
            label = "Red LED";
        };
    };
};
```

### Example 2: Add Button
```dts
/ {
    buttons {
        compatible = "gpio-keys";
        button0: button_0 {
            gpios = <&gpioc 13 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
            label = "User Button";
        };
    };
};
```

### Example 3: Add Child Node to Existing Peripheral
```dts
&i2c1 {
    status = "okay";
    clock-frequency = <I2C_BITRATE_FAST>;
    
    sensor: mpu6050@68 {
        compatible = "invensense,mpu6050";
        reg = <0x68>;
        int-gpios = <&gpiob 10 GPIO_ACTIVE_HIGH>;
    };
};
```

---

## Working with Aliases and Chosen

### Aliases
Aliases provide convenient names to reference nodes in code.

#### Adding Aliases
```dts
/ {
    aliases {
        led0 = &green_led;
        sw0 = &user_button;
        myuart = &uart2;
        mysensor = &sensor;
    };
};
```

#### Accessing in C Code
```c
#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
```

### Chosen Nodes
Chosen nodes specify system-level defaults.

#### Common Chosen Properties
```dts
/ {
    chosen {
        zephyr,console = &uart0;        // Console output device
        zephyr,shell-uart = &uart0;     // Shell UART
        zephyr,bt-uart = &uart1;        // Bluetooth UART
        zephyr,uart-mcumgr = &uart2;    // MCUmgr UART
        zephyr,sram = &sram0;           // SRAM region
        zephyr,flash = &flash0;         // Flash region
        zephyr,code-partition = &slot0_partition;  // Code storage
    };
};
```

---

## Common Property Types

### Integer Properties
```dts
clock-frequency = <48000000>;           // Single value
reg = <0x40013800 0x400>;              // Address and size
interrupts = <37 0>;                    // IRQ number and priority
gpio-reserved-ranges = <0 2>, <15 1>;  // Multiple ranges
```

### String Properties
```dts
label = "UART_0";
compatible = "st,stm32-uart";
status = "okay";  // or "disabled"
pinctrl-names = "default", "sleep";
```

### Phandle Properties
```dts
clocks = <&rcc STM32_CLOCK_BUS_APB2 0x00004000>;
gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
dmas = <&dma1 3 0x20440 0x0>;
```

### Array Properties
```dts
cs-gpios = <&gpioa 4 GPIO_ACTIVE_LOW>,
           <&gpiob 12 GPIO_ACTIVE_LOW>;
```

### Boolean Properties
```dts
hw-flow-control;  // Present = true
// Absent = false (just omit the property)
```

### Byte Strings
```dts
mac-address = [00 11 22 33 44 55];
```

---

## Practical Examples

### Example 1: Complete UART Configuration
```dts
// Enable UART2 with custom settings
&uart2 {
    status = "okay";
    current-speed = <115200>;
    pinctrl-0 = <&uart2_tx_pa2 &uart2_rx_pa3>;
    pinctrl-names = "default";
};

/ {
    chosen {
        zephyr,console = &uart2;
        zephyr,shell-uart = &uart2;
    };
    
    aliases {
        myuart = &uart2;
    };
};
```

### Example 2: SPI Device with Multiple Slaves
```dts
&spi1 {
    status = "okay";
    cs-gpios = <&gpioa 4 GPIO_ACTIVE_LOW>,
               <&gpiob 12 GPIO_ACTIVE_LOW>;
    
    spi_device0: spidev@0 {
        compatible = "vendor,spi-device";
        reg = <0>;
        spi-max-frequency = <1000000>;
    };
    
    spi_device1: spidev@1 {
        compatible = "vendor,spi-device";
        reg = <1>;
        spi-max-frequency = <2000000>;
    };
};
```

### Example 3: I2C Bus with Multiple Sensors
```dts
&i2c1 {
    status = "okay";
    clock-frequency = <I2C_BITRATE_FAST>;  // 400kHz
    
    temp_sensor: lm75@48 {
        compatible = "lm75";
        reg = <0x48>;
        label = "TEMP_SENSOR";
    };
    
    imu: mpu6050@68 {
        compatible = "invensense,mpu6050";
        reg = <0x68>;
        int-gpios = <&gpiob 10 GPIO_ACTIVE_HIGH>;
        label = "IMU";
    };
};

/ {
    aliases {
        temp-sensor = &temp_sensor;
        imu = &imu;
    };
};
```

### Example 4: PWM for Motor Control
```dts
&timers3 {
    status = "okay";
    
    pwm3: pwm {
        status = "okay";
        pinctrl-0 = <&tim3_ch1_pa6 &tim3_ch2_pa7>;
        pinctrl-names = "default";
    };
};

/ {
    pwm_motors {
        compatible = "pwm-leds";
        motor_left: motor_0 {
            pwms = <&pwm3 1 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
            label = "Left Motor";
        };
        motor_right: motor_1 {
            pwms = <&pwm3 2 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
            label = "Right Motor";
        };
    };
    
    aliases {
        motor-left = &motor_left;
        motor-right = &motor_right;
    };
};
```

### Example 5: Disabling Unused Peripherals
```dts
// Disable peripherals to save power
&usart3 {
    status = "disabled";
};

&spi2 {
    status = "disabled";
};

&adc1 {
    status = "disabled";
};

&can1 {
    status = "disabled";
};
```

### Example 6: Memory Partitions
```dts
&flash0 {
    partitions {
        compatible = "fixed-partitions";
        #address-cells = <1>;
        #size-cells = <1>;
        
        boot_partition: partition@0 {
            label = "mcuboot";
            reg = <0x00000000 0x00010000>;
        };
        
        slot0_partition: partition@10000 {
            label = "image-0";
            reg = <0x00010000 0x00070000>;
        };
        
        slot1_partition: partition@80000 {
            label = "image-1";
            reg = <0x00080000 0x00070000>;
        };
        
        storage_partition: partition@f0000 {
            label = "storage";
            reg = <0x000f0000 0x00010000>;
        };
    };
};

/ {
    chosen {
        zephyr,code-partition = &slot0_partition;
    };
};
```

---

## Best Practices

### 1. **Keep Overlays Organized**
```dts
// Group related configurations together
// 1. Peripheral configurations
&uart0 { ... };
&i2c1 { ... };
&spi1 { ... };

// 2. New node definitions
/ {
    leds { ... };
    buttons { ... };
};

// 3. Aliases and chosen
/ {
    aliases { ... };
    chosen { ... };
};
```

### 2. **Use Comments**
```dts
// UART0 configuration for console output
&uart0 {
    current-speed = <115200>;  // Standard baud rate
    status = "okay";            // Enable the peripheral
};
```

### 3. **Reference Board Files**
Check your board's default devicetree:
```bash
# Find your board's DTS file
find $ZEPHYR_BASE/boards -name "*.dts" | grep your_board

# View the file
cat $ZEPHYR_BASE/boards/vendor/board/board.dts
```

### 4. **Validate Your Overlay**
```bash
# Build to check for errors
west build -b your_board

# View generated devicetree
cat build/zephyr/zephyr.dts
```

### 5. **Use Devicetree Macros in Code**
```c
// Access nodes safely
#if DT_NODE_EXISTS(DT_ALIAS(led0))
    #define LED_NODE DT_ALIAS(led0)
    static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
#else
    #error "led0 alias not found"
#endif
```

### 6. **Common Pitfalls to Avoid**
- ❌ Forgetting semicolons at the end of properties
- ❌ Using wrong reference syntax (use `&label`, not `label`)
- ❌ Misspelling property names (devicetree is case-sensitive)
- ❌ Not enabling parent nodes (e.g., enable I2C bus before adding devices)
- ❌ Incorrect phandle syntax (use `<&node ...>`, not `&node`)

---

## Quick Reference Card

| Task | Syntax |
|------|--------|
| Modify property | `&node { property = <value>; };` |
| Add property | Same as modify |
| Delete property | `&node { /delete-property/ prop; };` |
| Create node | `/ { new_node { ... }; };` |
| Add alias | `/ { aliases { name = &node; }; };` |
| Set chosen | `/ { chosen { zephyr,console = &uart0; }; };` |
| Enable peripheral | `&node { status = "okay"; };` |
| Disable peripheral | `&node { status = "disabled"; };` |
| Reference GPIO | `gpios = <&port pin flags>;` |
| Reference clock | `clocks = <&rcc BUS ENABLE_BIT>;` |

---

## Additional Resources

- **Zephyr Devicetree Guide**: https://docs.zephyrproject.org/latest/build/dts/index.html
- **Devicetree Spec**: https://www.devicetree.org/specifications/
- **Zephyr Bindings**: `$ZEPHYR_BASE/dts/bindings/`
- **Board DTS Files**: `$ZEPHYR_BASE/boards/`

---

**Author**: GitHub Copilot  
**Last Updated**: December 4, 2025  
**Version**: 1.0  
**License**: Feel free to use and share this guide
