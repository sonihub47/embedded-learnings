## Challenge 2 — RTOS Pipeline Latency

**The problem:** A three-task pipeline — SPI read, image convert, image process — implemented with OS signals:

```c
// T1
SPI_transfer(...);
osSignalPost(T3, 0x12);

// T3
osSignalWait(0x12);
image_conversion(rxBuffer, NUM_COL, rgb565);
osSignalPost(T2, 0x22);

// T2
osSignalWait(0x22);
image_processing(rgb565);
```

**The bottleneck:** Single shared `rxBuffer`. Despite three RTOS tasks, execution is sequential — T1 blocks until T3 is done, T3 blocks until T2 is done. You're paying context switch overhead for zero parallelism.

**The fix:** Two mail queues with memory pools.

```
T1 → [mail_A] → T3 → [mail_B] → T2
```

T1 allocates a block from mail_A, SPI writes into it, posts the pointer. T3 picks it up, allocates a block from mail_B, converts directly into it, posts that pointer, frees the mail_A block. T2 picks up from mail_B, processes, frees.

In steady state T1, T3, T2 are all active simultaneously on different buffers.

**A few deliberate choices:**

*Zero copy in T3* — `image_conversion()` writes directly into the mail_B block. No local rgb565 buffer, no extra memcpy.

*Necessary copy in T1* — SPI driver owns `rxBuffer`, can't redirect it. The memcpy into the mail block here is unavoidable and called out in a comment.

*Queue depth* — sized from timing analysis of T_spi vs T_conv vs T_proc. If SPI outruns conversion, the queue absorbs the burst. When full, T1 blocks on `osMailAlloc()` — intentional backpressure.

*Watermarks* — in a production build, tracking free pool blocks at runtime gives early warning if the pipeline starts falling behind its timing budget.