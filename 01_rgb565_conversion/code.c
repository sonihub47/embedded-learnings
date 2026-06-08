#include <stdint.h>
#include <assert.h>
// Convert Raw Image Data to RGB 565 Format
// Input Data: 224 pixels (Each Pixel is 2B) (R G B)
// Each Pixel 
// [b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0]
// [B     B    B    B    B   G    G   G   G   G   G   R   R   R   R   R]
// Input Data Shape: 1 X 448 (raw image) 
// Output Data Shape: 1 X 772 (RGB565)
// [R0, R1, .......R223][G0, G1, ...... G223][B0, B1, ...... B223]
void image_conversion(const uint8_t* input, uint16_t size, uint8_t* output)
{ // code here
    
    // Observation: output data size should be 672 (224 * 3) and not 772 !?

    // NULL checks
    if(!input || !output)
        return;

    // raw size should be even (since "twice" of number of pixels)
    if(size % 2 != 0)
        return;    
        
    for(int byteIndex = 0; byteIndex < (size/2); byteIndex++)
    {
        // BLUE
        output[byteIndex + size] = (*input & 0xF8) >> 3; 

        // GREEN
        output[byteIndex + (size/2)] = (*input & 0x07) << 3;
        input++;
        output[byteIndex + (size/2)] |= (*input & 0xE0) >> 5;

        // RED
        output[byteIndex] = (*input & 0x1F);

        input++;    // to read the next pixel
    }
}
