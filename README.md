# Linux Kernel GPIO Character Device Driver

This repository contains a Loadable Kernel Module (LKM) implementing a character device driver for GPIO control using the modern `gpiod` API. It exposes hardware line manipulation to user space via a dedicated device node.

Made using the Raspberry Pi 3B's buil-in GPIO for proof of concept.

---

## Features

* **gpiod Architecture**: Built on descriptor-based consumer interfaces instead of direct, deprecated memory offsets.
* **Dynamic Node Generation**: Automatically instantiates the communication endpoint inside `/dev/` using kernel class and device creation APIs.
* **Run-time Configuration**: Supports custom pin assignments and virtualization offsets using module parameters.

---

## Module Parameters

The driver exposes two variables configurable during runtime initialization:

* `gpio_pin`: Specifies the physical hardware pin index (Default: `21`).
* `gpio_offset`: Sets the base offset virtualized by the kernel (Default: `512`).

---

## Deployment and Interaction

### 1. Load the Driver
Insert the compiled module (`.ko`) into the kernel infrastructure, optionally passing parameters (gpio_pin AND gpio_offset) to override defaults:

```bash
sudo insmod gpio.ko gpio_pin=21 gpio_offset=512
```

### 2. Verify Node Generation

  Confirm that the virtual subsystem interface was properly mounted by the kernel architecture:

```bash
ls -l /dev/gpio_tcc_dev
```

### 3. Trigger Hardware States

Stream input characters directly into the caracter device block to eveluate the driver's write logic sequences:

#### Drive logical HIGH (LED On):
```bash
echo "1" | sudo tee /dev/gpio_tcc_dev
```

#### Drive logical LOW (LED Off):
```bash
echo "0" | sudo tee /dev/gpio_tcc_dev
```

 ### 4. Inspect Kernel Transmissions

 You may now monitor system diagnostic and internal memory copy actions (copy_from_user) inside the kernel ring buffer logs:

 ```bash
 sudo dmesg | tail -n 5
 ```

 Or you can just see it in real through the -W flag in another terminal! (my personal favourite, using TMUX was a blast.)

 ```bash
 sudo dmesg -W
 ```

 ### 5. Terminate and Unload

 To safely decouple the driver, release registered major/minor allocations and force pin states back to low you can use the following:

 ```bash
 sudo rmmod gpio
 ```

 A fun little project which deepened my understanding of the world of drivers, kernel and old books! (Shoutout to Greg Kroah-Hartman and LDD3).

 