# Zephyr RTOS: Inter-Task Communication Guide

## Table of Contents
1. [Overview](#overview)
2. [Message Queues (k_msgq)](#message-queues-k_msgq)
3. [Mailboxes (k_mbox)](#mailboxes-k_mbox)
4. [Pipes (k_pipe)](#pipes-k_pipe)
5. [Work Queues (k_work / k_work_q)](#work-queues-k_work--k_work_q)
6. [FIFOs (k_fifo)](#fifos-k_fifo)
7. [LIFOs (k_lifo)](#lifos-k_lifo)
8. [Comparison Table](#comparison-table)
9. [Best Practices](#best-practices)

---

## Overview

Zephyr RTOS provides several kernel objects for inter-task communication (ITC). Each object is optimized for different data-passing patterns, latency requirements, and thread-safety guarantees. Choosing the right ITC primitive reduces overhead, simplifies synchronization, and improves system determinism.

---

## Message Queues (k_msgq)

A **message queue** stores a fixed number of fixed-size messages in a FIFO ring buffer. Producers enqueue messages; consumers dequeue them. The queue is thread-safe and ISR-safe.

### API Syntax

```c
#include <zephyr/kernel.h>

/* ── Static definition ───────────────────────────────────────────── */
K_MSGQ_DEFINE(my_msgq,          /* name                        */
              sizeof(uint32_t), /* message size in bytes        */
              10,               /* maximum number of messages   */
              4);               /* alignment of message buffer  */

/* ── Runtime definition ──────────────────────────────────────────── */
struct k_msgq my_msgq;
char __aligned(4) msgq_buffer[10 * sizeof(uint32_t)];

k_msgq_init(&my_msgq, msgq_buffer, sizeof(uint32_t), 10);

/* ── Send a message ──────────────────────────────────────────────── */
// Block indefinitely until space is available
int ret = k_msgq_put(&my_msgq, &data, K_FOREVER);

// Return immediately if queue is full
ret = k_msgq_put(&my_msgq, &data, K_NO_WAIT);

// Wait up to 100 ms
ret = k_msgq_put(&my_msgq, &data, K_MSEC(100));

/* ── Receive a message ───────────────────────────────────────────── */
uint32_t rx_data;
ret = k_msgq_get(&my_msgq, &rx_data, K_FOREVER);
ret = k_msgq_get(&my_msgq, &rx_data, K_NO_WAIT);
ret = k_msgq_get(&my_msgq, &rx_data, K_MSEC(100));

/* ── Peek without removing ───────────────────────────────────────── */
ret = k_msgq_peek(&my_msgq, &rx_data);

/* ── Queue status ────────────────────────────────────────────────── */
uint32_t used  = k_msgq_num_used_get(&my_msgq);
uint32_t free  = k_msgq_num_free_get(&my_msgq);

/* ── Purge all pending messages ──────────────────────────────────── */
k_msgq_purge(&my_msgq);
```

### Benefits
- **Simple, fixed-size protocol** — zero dynamic allocation; message size is known at compile time.
- **ISR-safe** — `k_msgq_put()` / `k_msgq_get()` can be called from interrupt handlers with `K_NO_WAIT`.
- **Built-in flow control** — sender blocks or times out if the queue is full.
- **Ring-buffer efficiency** — O(1) enqueue and dequeue with no fragmentation.

### Best Scenarios
| Scenario | Why Message Queues Fit |
|---|---|
| Sensor data pipeline (ISR → processing thread) | ISR-safe put; fixed sensor data size |
| Command/event dispatch between tasks | FIFO ordering preserves command sequence |
| Logging subsystem (producer → logger thread) | Natural back-pressure on overflow |
| State machine event passing | Each event is a small, fixed struct |

---

## Mailboxes (k_mbox)

A **mailbox** provides synchronous or asynchronous message exchange with optional message queuing. Unlike message queues, mailboxes support variable-size data and can perform a zero-copy transfer directly between the sender's and receiver's buffers when both rendezvous simultaneously.

### API Syntax

```c
#include <zephyr/kernel.h>

/* ── Static definition ───────────────────────────────────────────── */
K_MBOX_DEFINE(my_mbox);

/* ── Runtime definition ──────────────────────────────────────────── */
struct k_mbox my_mbox;
k_mbox_init(&my_mbox);

/* ── Send a message ──────────────────────────────────────────────── */
struct k_mbox_msg send_msg = {
    .size   = sizeof(my_data),   /* number of bytes to transfer    */
    .info   = 0x1234,            /* 32-bit app-defined type/info   */
    .tx_data = &my_data,         /* pointer to data (synchronous)  */
    .tx_target_thread = K_ANY,   /* send to any waiting receiver   */
};

// Synchronous send – block until receiver collects the message
int ret = k_mbox_put(&my_mbox, &send_msg, K_FOREVER);

// Asynchronous send – place in queue, return immediately
// tx_sem is given by the kernel when the receiver collects the
// message (or on error), so tx_sem must stay valid until then.
struct k_sem tx_sem;
k_sem_init(&tx_sem, 0, 1);
k_mbox_async_put(&my_mbox, &send_msg, &tx_sem);
// Wait for delivery confirmation (optional)
k_sem_take(&tx_sem, K_FOREVER);

/* ── Receive a message ───────────────────────────────────────────── */
struct k_mbox_msg recv_msg = {
    .size    = sizeof(my_data),  /* maximum bytes to accept        */
    .rx_source_thread = K_ANY,   /* accept from any sender         */
};
char recv_buf[64];

ret = k_mbox_get(&my_mbox, &recv_msg, recv_buf, K_FOREVER);
// recv_msg.size now contains the actual number of bytes received
// recv_msg.info contains the sender's info value

/* ── Retrieve data after a header-only receive ───────────────────── */
// First get only the header (pass NULL buffer)
ret = k_mbox_get(&my_mbox, &recv_msg, NULL, K_FOREVER);
// Then retrieve the actual payload
ret = k_mbox_data_get(&recv_msg, recv_buf);

/* ── Block copy with dynamic allocation ──────────────────────────── */
// 'block' receives a handle to the allocated memory region.
// 'my_resource_pool' is a k_heap or k_mem_slab used as the
// memory source; it must be initialised before this call.
struct k_mem_block block;
ret = k_mbox_data_block_get(&recv_msg, &block, &my_resource_pool, K_FOREVER);
```

### Benefits
- **Variable message size** — the sender specifies the exact byte count at runtime.
- **Zero-copy rendezvous** — when sender and receiver are simultaneously ready, data is copied directly between their buffers, skipping any intermediate queue storage.
- **Directed messaging** — a sender can target a specific receiver thread (`tx_target_thread`), and a receiver can filter by source thread (`rx_source_thread`).
- **Flexible synchronization** — choose between blocking synchronous sends and non-blocking asynchronous sends backed by a semaphore.

### Best Scenarios
| Scenario | Why Mailboxes Fit |
|---|---|
| Transferring large, variable-length payloads | Zero-copy path avoids extra memory copies |
| Request-response protocols between two known threads | Directed messaging guarantees delivery to the right thread |
| Audio/image frame handoff | Variable frame sizes; zero-copy minimizes latency |
| Tasks exchanging metadata headers before bulk data | Two-phase get (header then payload) |

---

## Pipes (k_pipe)

A **pipe** provides a byte-stream channel between a writer and a reader. Data is written and read in arbitrary byte amounts from a kernel-managed ring buffer. Pipes are suitable for streaming data where message boundaries are not important.

### API Syntax

```c
#include <zephyr/kernel.h>

/* ── Static definition ───────────────────────────────────────────── */
K_PIPE_DEFINE(my_pipe,    /* name             */
              256,        /* buffer size (bytes) */
              4);         /* alignment        */

/* ── Runtime definition ──────────────────────────────────────────── */
struct k_pipe my_pipe;
unsigned char pipe_buf[256];
k_pipe_init(&my_pipe, pipe_buf, sizeof(pipe_buf));

/* ── Write to the pipe ───────────────────────────────────────────── */
size_t bytes_written;
int ret = k_pipe_put(&my_pipe,
                     src_buf,          /* pointer to data to send   */
                     128,              /* number of bytes to write  */
                     &bytes_written,   /* actual bytes written      */
                     1,                /* minimum bytes to write    */
                     K_FOREVER);       /* timeout                   */

/* ── Read from the pipe ──────────────────────────────────────────── */
size_t bytes_read;
ret = k_pipe_get(&my_pipe,
                 dst_buf,             /* destination buffer         */
                 128,                 /* bytes requested            */
                 &bytes_read,         /* actual bytes read          */
                 1,                   /* minimum bytes to read      */
                 K_FOREVER);

/* ── Query available data / free space ───────────────────────────── */
size_t readable = k_pipe_read_avail(&my_pipe);
size_t writable = k_pipe_write_avail(&my_pipe);
```

### Benefits
- **Byte-stream semantics** — readers and writers are decoupled from message boundaries.
- **Configurable minimum transfer** — the `min_xfer` parameter prevents spurious wake-ups; a thread only unblocks when at least N bytes are available.
- **Single kernel buffer** — no per-message metadata overhead; efficient for continuous streams.
- **Bidirectional flow control** — both put and get honor timeouts so neither side can starve the other indefinitely.

### Best Scenarios
| Scenario | Why Pipes Fit |
|---|---|
| Serial / UART data streaming | Byte-stream naturally maps to character streams |
| Audio sample buffering | Continuous, boundary-less sample data |
| Protocol parsers consuming variable-length frames | Reader can request exactly the bytes it needs |
| IPC between threads at different data rates | Pipe acts as a rate-decoupling buffer |

---

## Work Queues (k_work / k_work_q)

A **work queue** is a dedicated thread that executes **work items** — callbacks submitted from other threads or ISRs. Work items are queued and processed sequentially, offloading time-consuming work from ISR context or high-priority threads.

### API Syntax

```c
#include <zephyr/kernel.h>

/* ════════════════════════════════════════════════════════════════════
 * Simple Work Items (k_work)
 * ════════════════════════════════════════════════════════════════════ */

/* ── Define and initialize ───────────────────────────────────────── */
struct k_work my_work;

void work_handler(struct k_work *work)
{
    // Work executed in the work queue thread context
    printk("Work item executed\n");
}

k_work_init(&my_work, work_handler);

/* ── Submit to the system work queue ────────────────────────────── */
k_work_submit(&my_work);

/* ── Submit to a custom work queue ──────────────────────────────── */
k_work_submit_to_queue(&my_work_q, &my_work);

/* ── Cancel a pending work item ─────────────────────────────────── */
int ret = k_work_cancel(&my_work);

/* ── Check work item state ───────────────────────────────────────── */
bool pending = k_work_is_pending(&my_work);

/* ════════════════════════════════════════════════════════════════════
 * Delayable Work Items (k_work_delayable)
 * ════════════════════════════════════════════════════════════════════ */
struct k_work_delayable my_dwork;

k_work_init_delayable(&my_dwork, work_handler);

// Submit after 500 ms delay
k_work_schedule(&my_dwork, K_MSEC(500));

// Submit to a specific queue after a delay
k_work_schedule_for_queue(&my_work_q, &my_dwork, K_MSEC(500));

// Reschedule (resets delay if already pending)
k_work_reschedule(&my_dwork, K_MSEC(200));

// Cancel delayable work
k_work_cancel_delayable(&my_dwork);

/* ════════════════════════════════════════════════════════════════════
 * Custom Work Queue (k_work_q)
 * ════════════════════════════════════════════════════════════════════ */
struct k_work_q my_work_q;

K_THREAD_STACK_DEFINE(my_wq_stack, 1024);

struct k_work_queue_config wq_cfg = {
    .name      = "my_wq",
    .no_yield  = false,
};

k_work_queue_init(&my_work_q);
k_work_queue_start(&my_work_q,
                   my_wq_stack,
                   K_THREAD_STACK_SIZEOF(my_wq_stack),
                   5,         /* priority */
                   &wq_cfg);

/* ── Drain the work queue (wait for all items to finish) ────────── */
bool running = k_work_queue_drain(&my_work_q, false);

/* ── Stop the work queue ─────────────────────────────────────────── */
k_work_queue_unplug(&my_work_q);
```

### Benefits
- **ISR offloading** — submit work from an ISR with `K_NO_WAIT` semantics; the handler runs in thread context where blocking and allocation are allowed.
- **Serialized execution** — items in the same queue run one at a time, eliminating data races without explicit locking.
- **Delayable items** — schedule callbacks at a future time without a dedicated thread or timer callback doing the real work.
- **System work queue** — Zephyr provides a ready-to-use system work queue (`k_sys_work_q`), reducing boilerplate.
- **Priority control** — custom queues can be created at any priority to control scheduling relative to other threads.

### Best Scenarios
| Scenario | Why Work Queues Fit |
|---|---|
| GPIO / interrupt debouncing | ISR submits work; handler runs 50 ms later via `k_work_schedule` |
| Deferred driver I/O completion | ISR signals completion; slow processing happens off the ISR stack |
| Periodic maintenance tasks | `k_work_reschedule` provides timer-like behavior in thread context |
| Event-driven state machines | Each event becomes a work item, serialized execution is lock-free |
| Sensor polling at fixed intervals | Delayable work replaces a dedicated polling thread |

---

## FIFOs (k_fifo)

A **FIFO** is a singly-linked, first-in-first-out queue of arbitrary data items. Unlike message queues, the items themselves are linked together in memory — the kernel stores a pointer to each item rather than copying data. This makes FIFOs a zero-copy, allocation-free queue for items already residing in memory.

### API Syntax

```c
#include <zephyr/kernel.h>

/* Items placed in a FIFO MUST have a reserved pointer field
 * as their first member.                                          */
struct my_item {
    void  *fifo_reserved;   /* required first field               */
    int    value;
    char   data[32];
};

/* ── Static definition ───────────────────────────────────────────── */
K_FIFO_DEFINE(my_fifo);

/* ── Runtime definition ──────────────────────────────────────────── */
struct k_fifo my_fifo;
k_fifo_init(&my_fifo);

/* ── Enqueue a single item ───────────────────────────────────────── */
struct my_item item = { .value = 42 };
k_fifo_put(&my_fifo, &item);

/* ── Enqueue a list of items atomically ─────────────────────────── */
// Build a singly-linked list first, then hand it to the FIFO.
sys_slist_t pending_list;
sys_slist_init(&pending_list);

struct my_item a = { .value = 1 };
struct my_item b = { .value = 2 };
sys_slist_append(&pending_list, (sys_snode_t *)&a);
sys_slist_append(&pending_list, (sys_snode_t *)&b);

k_fifo_put_list(&my_fifo, &pending_list);

// Alternatively, provide head and tail pointers directly
k_fifo_put_slist(&my_fifo, &pending_list);

/* ── Dequeue (blocking) ──────────────────────────────────────────── */
struct my_item *rx = (struct my_item *)k_fifo_get(&my_fifo, K_FOREVER);
rx = (struct my_item *)k_fifo_get(&my_fifo, K_NO_WAIT);
rx = (struct my_item *)k_fifo_get(&my_fifo, K_MSEC(100));

/* ── Peek at head without removing ──────────────────────────────── */
struct my_item *peeked = (struct my_item *)k_fifo_peek_head(&my_fifo);
struct my_item *tail   = (struct my_item *)k_fifo_peek_tail(&my_fifo);

/* ── Check if empty ──────────────────────────────────────────────── */
bool empty = k_fifo_is_empty(&my_fifo);
```

### Benefits
- **Zero-copy** — only a pointer is queued; the data stays in the producer's buffer until the consumer uses it.
- **No data size limit** — items can be any size because the queue stores pointers, not copies.
- **ISR-safe** — `k_fifo_put()` is callable from ISRs for lock-free producer scenarios.
- **Atomic list enqueue** — `k_fifo_put_list()` adds an entire pre-built linked list in one atomic operation.

### Best Scenarios
| Scenario | Why FIFOs Fit |
|---|---|
| Memory-pool–based message passing | Items are allocated from a pool and freed after consumption |
| Network packet queuing | Large, variable-size packets passed by pointer |
| Event objects managed by a memory pool | Zero-copy avoids copying large structs |
| ISR → thread handoff of pre-allocated buffers | ISR allocates from pool, enqueues pointer, thread processes and frees |

---

## LIFOs (k_lifo)

A **LIFO** is a singly-linked, last-in-first-out stack of arbitrary data items. Like FIFOs, items must have a reserved pointer as their first member. LIFOs are useful when the most-recently added item should be processed first (stack semantics).

### API Syntax

```c
#include <zephyr/kernel.h>

/* Items placed in a LIFO MUST have a reserved pointer field
 * as their first member (same requirement as FIFO).               */
struct my_item {
    void  *lifo_reserved;   /* required first field               */
    int    value;
};

/* ── Static definition ───────────────────────────────────────────── */
K_LIFO_DEFINE(my_lifo);

/* ── Runtime definition ──────────────────────────────────────────── */
struct k_lifo my_lifo;
k_lifo_init(&my_lifo);

/* ── Push an item ────────────────────────────────────────────────── */
struct my_item item = { .value = 10 };
k_lifo_put(&my_lifo, &item);

/* ── Pop an item (blocking) ──────────────────────────────────────── */
struct my_item *rx = (struct my_item *)k_lifo_get(&my_lifo, K_FOREVER);
rx = (struct my_item *)k_lifo_get(&my_lifo, K_NO_WAIT);
rx = (struct my_item *)k_lifo_get(&my_lifo, K_MSEC(100));
```

### Benefits
- **Stack (LIFO) ordering** — the most recently added item is processed first, providing temporal locality.
- **Zero-copy** — pointer-based, identical to FIFO in memory efficiency.
- **ISR-safe** — `k_lifo_put()` is callable from ISRs.
- **Cache-friendly** — recently pushed items are likely still in CPU cache when popped.

### Best Scenarios
| Scenario | Why LIFOs Fit |
|---|---|
| Free-list / memory-pool management | Reuse the most recently freed buffer (cache warm) |
| Undo/history stacks in UI tasks | Natural LIFO semantics for history |
| Priority-less work where recency matters | Latest data is processed before older stale data |
| Recursive work decomposition | Push sub-tasks and pop in reverse order |

---

## Comparison Table

| Feature | Message Queue | Mailbox | Pipe | Work Queue | FIFO | LIFO |
|---|---|---|---|---|---|---|
| **Data model** | Fixed-size msgs | Variable-size msgs | Byte stream | Callback functions | Variable-size items (ptr) | Variable-size items (ptr) |
| **Ordering** | FIFO | FIFO | FIFO | FIFO (per queue) | FIFO | LIFO |
| **Copy semantics** | Copy into kernel buffer | Copy or zero-copy rendezvous | Copy into ring buffer | N/A (function pointer) | Zero-copy (pointer) | Zero-copy (pointer) |
| **ISR-safe put** | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| **Directed messaging** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| **Variable message size** | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Built-in delay support** | ❌ | ❌ | ❌ | ✅ (`k_work_delayable`) | ❌ | ❌ |
| **Typical use** | Commands, events | Large payloads, RPC | Byte streams | Deferred/periodic work | Packet queuing | Free lists, undo stacks |

---

## Best Practices

1. **Match semantics to your data model.**
   - Use a **message queue** for small, fixed-size events or commands.
   - Use a **mailbox** when message sizes vary or you need directed, synchronous handshakes.
   - Use a **pipe** for continuous byte streams with no natural framing.
   - Use a **FIFO** or **LIFO** when the item already lives in memory and copying is wasteful.
   - Use a **work queue** to defer processing out of ISR or high-priority thread context.

2. **Prefer `K_MSGQ_DEFINE` / `K_PIPE_DEFINE` / `K_FIFO_DEFINE`** (compile-time macros) over runtime initialization to avoid startup ordering issues and reduce RAM fragmentation.

3. **Always check return values.** `k_msgq_put()`, `k_pipe_put()`, and similar functions return `-EAGAIN` on timeout and `-ENOMSG` when no message is available. Ignoring these values causes silent data loss.

4. **Size your buffers for worst-case burst.** Use `k_msgq_num_free_get()` or `k_pipe_write_avail()` in diagnostic code to detect near-overflow conditions during testing.

5. **Avoid blocking inside ISRs.** All ISR-safe put functions must be called with `K_NO_WAIT`. Blocking timeouts in ISR context will cause a kernel panic.

6. **Use the system work queue (`k_sys_work_q`) for lightweight deferred work.** Reserve custom work queues for tasks that need a specific stack size, priority, or isolation from system work.

7. **Align message buffers.** Message queues require that the buffer alignment matches the alignment of the message type. Use `__aligned(N)` or specify alignment in `K_MSGQ_DEFINE` to avoid hard faults on architectures with strict alignment requirements.

8. **Combine ITC primitives when needed.** For example, a FIFO can carry pre-allocated buffers while a semaphore signals the consumer — this pattern avoids copying large payloads while still providing flow control.
