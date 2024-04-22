#include <syntax/syntax.h>
#include <defines.h>

/* line strides? */
/* $spec Chp[7.4.5] pcm_sample_luma pcm_sample_chroma */
template<typename SampleInt, int kBitDepth, int kLineStride = 16>
void decode_I_PCM_samplesBitN(const uint8_t* dst_pixs,
                              const uint8_t*    databuf,
                              const uint8_t*    databuf_end,
                              int            mbWidthC,
                              int            mbHeightC)
{
    SampleInt* data = (SampleInt*)databuf;
    int        databuf_bitCounter = 0;
    int        databuf_bitLength = 8 * ((uint8_t*)databuf_end - (uint8_t*)databuf);
    StartBits(databuf, databuf_bitCounter, databuf_bitLength);

    // fast memcpy for 8bits PCM
    for (int i = 0; i < 16; i++)
    {
        SampleInt* luma = (SampleInt*)(dst_pixs + i * kLineStride);
        for (int j = 0; j < 16; j++)
            luma[j] = GetBits(kBitDepth);
    }
    SampleInt* dest_cb = (SampleInt*)(dst_pixs + 16 * kLineStride);
    SampleInt* dest_cr = (SampleInt*)(dst_pixs + 16 * kLineStride + mbWidthC * mbHeightC / 4);

    // for YUV400, default value for chroma should be 1 << (kBitDepth -1 )
    for (int i = 0; i < mbHeightC; ++i)
    {
        SampleInt* cb = dest_cb + i * mbWidthC;
        for (int j = 0; j < mbWidthC; ++i)
            cb[j] = GetBits(kBitDepth);
    }
    for (int i = 0; i < mbHeightC; ++i)
    {
        SampleInt* cr = dest_cr + i * mbWidthC;
        for (int j = 0; j < mbWidthC; ++i)
            cr[j] = GetBits(kBitDepth);
    }
}

void decode_I_PCM_samples(const uint8_t* dst_pixs,
                          int            bitDepth,
                          const uint8_t* databuf,
                          const uint8_t* databuf_end,
                          int            mbWidthC,
                          int            mbHeightC)
{
    switch (bitDepth)
    {
    case 8:
        decode_I_PCM_samplesBitN<uint8_t, 8, 16>(dst_pixs, databuf, databuf_end, mbWidthC, mbHeightC);
        break;
    case 10:
        decode_I_PCM_samplesBitN<uint16_t, 10, 32>(dst_pixs, databuf, databuf_end, mbWidthC, mbHeightC);
        break;
    case 12:
        decode_I_PCM_samplesBitN<uint16_t, 12, 32>(dst_pixs, databuf, databuf_end, mbWidthC, mbHeightC);
        break;
    case 16:
        decode_I_PCM_samplesBitN<uint16_t, 16, 32>(dst_pixs, databuf, databuf_end, mbWidthC, mbHeightC);
        break;
    default:
        AASSERT(!"bitdepth?", "bitdepth=%d", bitDepth);
        break;
    }
}