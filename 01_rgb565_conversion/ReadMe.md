## Challenge 1 — RGB565 Image Conversion

**The problem:** Convert a packed 16-bit raw image into planar RGB565 format.

```c
void image_conversion(const uint8_t* input, uint16_t size, uint8_t* output);
```

Input is 224 pixels, 2 bytes each (448 bytes). Each pixel packs B, G, R into 16 bits:
```
[B4 B3 B2 B1 B0 G5 G4 G3 | G2 G1 G0 R4 R3 R2 R1 R0]
```

Output is planar — all R bytes, then all G bytes, then all B bytes:
```
[R0..R223][G0..G223][B0..B223]  → 672 bytes
```

> The original problem states 772 bytes output — looks like a typo. 224 pixels × 3 channels = 672 bytes.

**Constraints worth noting:**
- Embedded target — no stack-allocated intermediate buffers
- `size` is in bytes, not pixels
- `input` is read-only, enforced via `const`

**Approach:** Single loop over pixels. Each iteration extracts R, G, B via mask+shift and writes directly into the three planar regions using index offsets. No dynamic allocation, no intermediate buffers.

---
