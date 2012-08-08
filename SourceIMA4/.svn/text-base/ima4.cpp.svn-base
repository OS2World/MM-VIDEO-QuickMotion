#define INCL_32
#include <os2.h>

#include <memory.h>

#include "ima4.h"

struct adpcm_state {
    LONG valprev; /* Previous output value */
    signed char index; /* Index into stepsize table */
    LONG step; /* Stepsize table value */
};

/* Intel ADPCM step variation table */
static LONG indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static LONG stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


#define ima4Decode(valprev, delta, index, step)       \
{                                                     \
  LONG vpdiff, sign;                                  \
  /* Find new index value (for later) */              \
  index += indexTable[delta];                         \
  if (index < 0) index = 0;                           \
  else if (index > 88) index = 88;                    \
  /* Separate sign and magnitude */                   \
  sign = delta & 8;                                   \
  delta = delta & 7;                                  \
  /* Compute difference and new predicted value */    \
  vpdiff = step >> 3;                                 \
  if (delta & 4)  vpdiff += step;                     \
  if (delta & 2)  vpdiff += step>>1;                  \
  if (delta & 1)  vpdiff += step>>2;                  \
  if (sign)       valprev -= vpdiff;                  \
  else            valprev += vpdiff;                  \
  if (valprev > 32767)            valprev = 32767;    \
  else if (valprev < -32768)      valprev = -32768;   \
  /* update step value */                             \
  step = stepsizeTable[index];                        \
}

void ima4Expand(PUCHAR soundBuffer, USHORT channels, ULONG blocks)
{
  struct adpcm_state leftState;
  struct adpcm_state rightState;
  LONG nybble0L, nybble1L, nybble0R, nybble1R;

  /* Copy compressed sound to a temporary buffer.  Decompressed goes back in original. */
//  LONG compressedBufferSize = channels*blocks*IMA4_BLOCK_SIZE;
//  PUCHAR compressedBuffer = new UCHAR[compressedBufferSize];
//  memcpy(compressedBuffer, soundBuffer, compressedBufferSize);
  PUCHAR currentCompL = soundBuffer;
  PUCHAR currentCompR = soundBuffer + IMA4_BLOCK_SIZE;

  LONG channelSamples = channels*blocks*IMA4_BLOCK_SAMPLES;
  PSHORT decompressedBuffer = new SHORT[channelSamples];
  PSHORT currentDecomp = decompressedBuffer;

  for (; blocks > 0; blocks--)
  {
    /* Process left (mono) block header */
    leftState.valprev = *currentCompL++ << 8;
    leftState.valprev |= *currentCompL++;
    leftState.index = leftState.valprev & 0x007F;
    if (leftState.index > 88)
      leftState.index = 88;
    leftState.step = stepsizeTable[leftState.index];
    leftState.valprev &= 0xFF00;
    if (leftState.valprev & 0x8000)
      leftState.valprev = leftState.valprev - 0x10000;

    if (channels == 2)
    {
      /* Process right block header */
      rightState.valprev = *currentCompR++ << 8;
      rightState.valprev |= *currentCompR++;
      rightState.index = rightState.valprev & 0x007F;
      if (rightState.index > 88)
        rightState.index = 88;
      rightState.step = stepsizeTable[rightState.index];
      rightState.valprev &= 0xFF00;
      if (rightState.valprev & 0x8000)
        rightState.valprev = rightState.valprev - 0x10000;
    }

    for (LONG i = IMA4_BLOCK_SAMPLES / 2; i > 0; i--)
    {
      /* Process compressed samples */
      nybble0L = *currentCompL++;
      nybble1L = (nybble0L >> 4) & 0x0F;
      nybble0L &= 0x0F;
      ima4Decode(leftState.valprev, nybble0L, leftState.index, leftState.step);
      *currentDecomp++ = leftState.valprev;

      if (channels == 2)
      {
        nybble0R = *currentCompR++;
        nybble1R = (nybble0R >> 4) & 0x0F;
        nybble0R &= 0x0F;
        ima4Decode(rightState.valprev, nybble0R, rightState.index, rightState.step);
        *currentDecomp++ = rightState.valprev;
      }

      ima4Decode(leftState.valprev, nybble1L, leftState.index, leftState.step);
      *currentDecomp++ = leftState.valprev;

      if (channels == 2)
      {
        ima4Decode(rightState.valprev, nybble1R, rightState.index, rightState.step);
        *currentDecomp++ = rightState.valprev;
      }
    }
    if (channels == 2)
    {
      currentCompL += IMA4_BLOCK_SIZE;
      currentCompR += IMA4_BLOCK_SIZE;
    }
  }
  memcpy(soundBuffer, decompressedBuffer, channelSamples*sizeof(SHORT));
 // delete[] compressedBuffer;
  delete[] decompressedBuffer;
}






