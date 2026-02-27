# Zephyr Software Timer Complete Guide

## Table of Contents
1. [Overview](#overview)
2. [Basic API Reference](#basic-api-reference)
3. [Timer Initialization](#timer-initialization)
4. [Timer Start Parameters](#timer-start-parameters)
5. [Callback Functions](#callback-functions)
6. [Configuration Options](#configuration-options)
7. [Complete Examples](#complete-examples)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

## Overview

Zephyr software timers (`k_timer`) provide a way to schedule callback functions to execute after specified time intervals. They are kernel objects that can be used for one-shot or periodic operations.

**Key Features:**
- Hardware independent
- Non-blocking operation
- Callback execution in ISR context
- Support for both one-shot and periodic timers
- Microsecond precision (depends on system tick configuration)

## Basic API Reference

### Timer Declaration
```c
#include <zephyr/kernel.h>

// Runtime declaration
struct k_timer my_timer;

// Compile-time declaration with initialization
K_TIMER_DEFINE(my_static_timer, expiry_fn, stop_fn);
```

### Core Functions
```c
// Initialize timer
void k_timer_init(struct k_timer *timer, 
                  k_timer_expiry_t expiry_fn, 
                  k_timer_stop_t stop_fn);

// Start timer
void k_timer_start(struct k_timer *timer, 
                   k_timeout_t duration, 
                   k_timeout_t period);

// Stop timer
void k_timer_stop(struct k_timer *timer);

// Check timer status
uint32_t k_timer_status_get(struct k_timer *timer);
k_timeout_t k_timer_remaining_get(struct k_timer *timer);
```

### Time Macros
```c
K_NO_WAIT          // 0 timeout
K_FOREVER          // Infinite timeout
K_MSEC(ms)         // Milliseconds
K_SECONDS(s)       // Seconds  
K_MINUTES(m)       // Minutes
K_HOURS(h)         // Hours
K_TICKS(n)         // System ticks
```

## Timer Initialization

### Method 1: Runtime Initialization
```c
struct k_timer my_timer;

void expiry_callback(struct k_timer *timer) {
    printk("Timer expired!\n");
}

void stop_callback(struct k_timer *timer) {
    printk("Timer stopped!\n");
}

int main(void) {
    k_timer_init(&my_timer, expiry_callback, stop_callback);
    // Timer is now ready to use
}
```

### Method 2: Compile-time Definition
```c
void expiry_callback(struct k_timer *timer) {
    printk("Timer expired!\n");
}

void stop_callback(struct k_timer *timer) {
    printk("Timer stopped!\n");
}

// Define timer at compile time
K_TIMER_DEFINE(my_static_timer, expiry_callback, stop_callback);

int main(void) {
    // Timer is already initialized, ready to start
    k_timer_start(&my_static_timer, K_SECONDS(1), K_SECONDS(1));
}
```

## Timer Start Parameters

The `k_timer_start()` function takes two timing parameters that control timer behavior:

### Parameter Breakdown
```c
k_timer_start(timer, duration, period);
```
- **duration**: Initial delay before first expiry
- **period**: Repetition interval (0 = one-shot)

### Timer Behavior Patterns

| Duration | Period | Behavior | Use Case |
|----------|---------|----------|----------|
| `K_SECONDS(5)` | `K_NO_WAIT` | Fire once after 5s | Delayed one-shot |
| `K_NO_WAIT` | `K_SECONDS(2)` | Fire immediately, then every 2s | Immediate periodic |
| `K_SECONDS(1)` | `K_SECONDS(2)` | Fire after 1s, then every 2s | Delayed periodic |
| `K_MSEC(100)` | `K_MSEC(100)` | Fire after 100ms, repeat every 100ms | Fast periodic |

### Detailed Examples

#### 1. One-Shot Timer (Fire Once)
```c
// Wait 3 seconds, fire once, then stop
k_timer_start(&timer, K_SECONDS(3), K_NO_WAIT);

Timeline:
0s -------- 3s [FIRE] -------- (timer inactive)
```

#### 2. Immediate Periodic Timer
```c
// Fire immediately, then every 2 seconds
k_timer_start(&timer, K_NO_WAIT, K_SECONDS(2));

Timeline:
[FIRE] -- 2s [FIRE] -- 2s [FIRE] -- 2s [FIRE] ...
```

#### 3. Delayed Periodic Timer
```c
// Wait 1 second, then fire every 2 seconds
k_timer_start(&timer, K_SECONDS(1), K_SECONDS(2));

Timeline:
0s - 1s [FIRE] -- 2s [FIRE] -- 2s [FIRE] -- 2s [FIRE] ...
```

#### 4. Synchronized Periodic Timer
```c
// Standard periodic: same initial delay and period
k_timer_start(&timer, K_SECONDS(5), K_SECONDS(5));

Timeline:
0s ---- 5s [FIRE] ---- 5s [FIRE] ---- 5s [FIRE] ...
```

## Callback Functions

### Expiry Callback
Executed when timer expires. Called from ISR context.

```c
void timer_expiry_handler(struct k_timer *timer)
{
    // IMPORTANT: Keep this function short and fast
    // Avoid blocking operations
    
    printk("Timer %p expired at %u ms\n", timer, k_uptime_get_32());
    
    // You can restart the timer if needed
    // k_timer_start(timer, K_SECONDS(10), K_NO_WAIT);
}
```

### Stop Callback (Optional)
Executed when timer is explicitly stopped via `k_timer_stop()`.

```c
void timer_stop_handler(struct k_timer *timer)
{
    printk("Timer %p was stopped\n", timer);
}
```

### ISR Context Limitations
Since callbacks run in ISR context, avoid:
- `k_sleep()` or blocking calls
- Long-running computations
- Large stack usage
- Non-ISR-safe kernel APIs

## Configuration Options

### Kconfig Settings (prj.conf)
```conf
# Basic kernel support (required)
CONFIG_KERNEL=y

# System clock configuration
CONFIG_SYS_CLOCK_TICKS_PER_SEC=1000    # 1ms tick resolution

# Optional: Tickless kernel for power saving
CONFIG_TICKLESS_KERNEL=y

# Optional: 64-bit timeout support for long durations
CONFIG_TIMEOUT_64BIT=y

# Optional: Userspace support
CONFIG_USERSPACE=y

# Optional: Enable printk for debugging
CONFIG_PRINTK=y
```

### System Clock Impact
- Higher `CONFIG_SYS_CLOCK_TICKS_PER_SEC` = better timer precision
- Lower values = better power efficiency
- Common values: 100, 1000, 10000 Hz

## Complete Examples

### Example 1: Basic Periodic Timer
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct k_timer heartbeat_timer;
static int counter = 0;

void heartbeat_expiry(struct k_timer *timer)
{
    printk("Heartbeat #%d at %u ms\n", ++counter, k_uptime_get_32());
}

int main(void)
{
    printk("Heartbeat Timer Example\n");
    
    k_timer_init(&heartbeat_timer, heartbeat_expiry, NULL);
    k_timer_start(&heartbeat_timer, K_SECONDS(1), K_SECONDS(1));
    
    while (1) {
        k_sleep(K_SECONDS(10));
        printk("Main thread alive\n");
    }
    
    return 0;
}
```

### Example 2: Multiple Timers with Different Rates
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct k_timer fast_timer, slow_timer;

void fast_expiry(struct k_timer *timer)
{
    printk("FAST: %u ms\n", k_uptime_get_32());
}

void slow_expiry(struct k_timer *timer)
{
    printk("SLOW: %u ms\n", k_uptime_get_32());
}

int main(void)
{
    printk("Multi-Timer Example\n");
    
    k_timer_init(&fast_timer, fast_expiry, NULL);
    k_timer_init(&slow_timer, slow_expiry, NULL);
    
    // Fast timer: every 500ms
    k_timer_start(&fast_timer, K_MSEC(500), K_MSEC(500));
    
    // Slow timer: every 3 seconds
    k_timer_start(&slow_timer, K_SECONDS(3), K_SECONDS(3));
    
    while (1) {
        k_sleep(K_FOREVER);
    }
    
    return 0;
}
```

### Example 3: One-Shot Timer Chain
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct k_timer sequence_timer;
static int sequence_step = 0;

void sequence_expiry(struct k_timer *timer)
{
    switch (sequence_step++) {
        case 0:
            printk("Step 1: Initialize (1s)\n");
            k_timer_start(timer, K_SECONDS(1), K_NO_WAIT);
            break;
        case 1:
            printk("Step 2: Configure (2s)\n");
            k_timer_start(timer, K_SECONDS(2), K_NO_WAIT);
            break;
        case 2:
            printk("Step 3: Execute (500ms)\n");
            k_timer_start(timer, K_MSEC(500), K_NO_WAIT);
            break;
        case 3:
            printk("Step 4: Complete!\n");
            // Sequence finished
            break;
    }
}

int main(void)
{
    printk("Timer Sequence Example\n");
    
    k_timer_init(&sequence_timer, sequence_expiry, NULL);
    k_timer_start(&sequence_timer, K_MSEC(100), K_NO_WAIT); // Start sequence
    
    while (1) {
        k_sleep(K_SECONDS(1));
    }
    
    return 0;
}
```

### Example 4: Timer with User Data
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct timer_context {
    const char *name;
    int max_count;
    int current_count;
};

struct k_timer configurable_timer;
struct timer_context timer_ctx = {"MyTimer", 5, 0};

void configurable_expiry(struct k_timer *timer)
{
    struct timer_context *ctx = k_timer_user_data_get(timer);
    
    ctx->current_count++;
    printk("%s: %d/%d\n", ctx->name, ctx->current_count, ctx->max_count);
    
    if (ctx->current_count >= ctx->max_count) {
        printk("%s completed!\n", ctx->name);
        k_timer_stop(timer);
    }
}

int main(void)
{
    printk("Timer with User Data Example\n");
    
    k_timer_init(&configurable_timer, configurable_expiry, NULL);
    k_timer_user_data_set(&configurable_timer, &timer_ctx);
    k_timer_start(&configurable_timer, K_SECONDS(1), K_SECONDS(1));
    
    while (1) {
        k_sleep(K_SECONDS(10));
    }
    
    return 0;
}
```

## Best Practices

### 1. Keep Callbacks Short
```c
// ❌ BAD: Long-running operation in callback
void bad_expiry(struct k_timer *timer)
{
    for (int i = 0; i < 1000000; i++) {
        // Complex computation - blocks ISR context
    }
}

// ✅ GOOD: Signal work thread from callback
K_SEM_DEFINE(work_sem, 0, 1);

void good_expiry(struct k_timer *timer)
{
    k_sem_give(&work_sem); // Quick signal to work thread
}

void work_thread(void)
{
    while (1) {
        k_sem_take(&work_sem, K_FOREVER);
        // Do complex work here in thread context
    }
}
```

### 2. Proper Timer Management
```c
// ✅ GOOD: Check if timer is running before stopping
void safe_timer_stop(struct k_timer *timer)
{
    if (k_timer_remaining_get(timer).ticks > 0) {
        k_timer_stop(timer);
    }
}

// ✅ GOOD: Reset timer state properly
void reset_timer(struct k_timer *timer)
{
    k_timer_stop(timer);
    // Reset any associated state
    counter = 0;
    k_timer_start(timer, K_SECONDS(1), K_SECONDS(1));
}
```

### 3. Timer Synchronization
```c
// Use timers for periodic tasks that don't require exact synchronization
// For precise timing, consider hardware timers or PWM peripherals

struct k_timer stats_timer;

void print_stats(struct k_timer *timer)
{
    // System statistics - doesn't need microsecond precision
    printk("Uptime: %u seconds\n", k_uptime_get_32() / 1000);
}
```

### 4. Resource Management
```c
// ✅ GOOD: Clean up timers before shutdown
void cleanup_timers(void)
{
    k_timer_stop(&heartbeat_timer);
    k_timer_stop(&stats_timer);
    // Timers are automatically cleaned up when out of scope
}
```

## Troubleshooting

### Common Issues

#### 1. Timer Not Firing
```c
// Check: Is timer initialized?
k_timer_init(&my_timer, expiry_func, NULL);

// Check: Is timer started?
k_timer_start(&my_timer, K_SECONDS(1), K_NO_WAIT);

// Check: Is expiry function defined correctly?
void expiry_func(struct k_timer *timer) { /* ... */ }
```

#### 2. Inconsistent Timing
```c
// Issue: Wrong parameter order
k_timer_start(&timer, K_SECONDS(1), K_SECONDS(100)); // First fires at 1s, then every 100s

// Fix: Use consistent periods
k_timer_start(&timer, K_SECONDS(1), K_SECONDS(1)); // Fires every 1s
```

#### 3. System Crashes in Callback
```c
// ❌ Cause: Blocking in ISR context
void bad_callback(struct k_timer *timer)
{
    k_sleep(K_MSEC(10)); // NEVER do this!
}

// ✅ Solution: Use work queue
K_WORK_DEFINE(my_work, work_handler);

void good_callback(struct k_timer *timer)
{
    k_work_submit(&my_work);
}

void work_handler(struct k_work *work)
{
    k_sleep(K_MSEC(10)); // OK in work queue context
}
```

#### 4. Timer Precision Issues
```c
// Check system tick configuration in prj.conf
CONFIG_SYS_CLOCK_TICKS_PER_SEC=1000  // 1ms resolution

// For high precision, consider:
CONFIG_SYS_CLOCK_TICKS_PER_SEC=10000 // 0.1ms resolution
```

### Debugging Tips

#### 1. Add Debug Prints
```c
void debug_expiry(struct k_timer *timer)
{
    static uint32_t last_time = 0;
    uint32_t current_time = k_uptime_get_32();
    
    printk("Timer: current=%u, delta=%u\n", 
           current_time, current_time - last_time);
    
    last_time = current_time;
}
```

#### 2. Monitor Timer Status
```c
void check_timer_status(struct k_timer *timer)
{
    k_timeout_t remaining = k_timer_remaining_get(timer);
    printk("Timer remaining: %d ticks\n", remaining.ticks);
}
```

#### 3. Use Timer Stats
```c
void timer_statistics(void)
{
    printk("System uptime: %u ms\n", k_uptime_get_32());
    printk("System ticks: %u\n", k_uptime_ticks());
}
```

---

## Summary

Zephyr software timers provide a flexible, efficient way to schedule periodic or one-shot operations. Key points to remember:

- Use `k_timer_init()` for runtime setup or `K_TIMER_DEFINE()` for compile-time
- First parameter of `k_timer_start()` is initial delay, second is repetition period  
- Keep callback functions short and fast (ISR context)
- Use appropriate time macros (`K_SECONDS()`, `K_MSEC()`, etc.)
- Consider system tick configuration for timing precision
- Use work queues for complex operations triggered by timers

For hardware-specific timing requirements or high-precision needs, consider using dedicated hardware timers instead of kernel software timers.