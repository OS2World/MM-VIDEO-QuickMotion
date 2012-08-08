#pragma inline

struct RGB
{
  char bBlue;
  char bGreen;
  char bRed;
};
#include "..\qfwpanof.hpp"

extern "C" __syscall void renderFrame(RendererSetup* setup)
{
  fixed u = 0;
  unsigned long middle;

  RGB *baseTopLeft, *baseBottomLeft, *baseTopRight, *baseBottomRight;
  //Initial values seem strange because they will be incremented right away
  asm
  {
    mov eax, [setup.scanLineBytes]
    shr eax, 1
    mov middle, eax
    add eax, [setup.frameBuffer]
    mov baseTopLeft, eax
    sub eax, [setup.scanLineBytes]
    mov baseBottomLeft, eax
    sub eax, 3
    mov baseBottomRight, eax
    add eax, [setup.scanLineBytes]
    mov baseTopRight, eax
  }

  for (int x = middle; x; x-=sizeof(RGB))
  {
    fixed u2T2T1;
    RGB *frameSpotTopLeft, *frameSpotTopRight, *frameSpotBottomLeft, *frameSpotBottomRight;
    // u2T2T1 = setup->u * setup->u + setup->tan2Tilt1;
    asm
    {
      mov eax, u
      imul dword ptr u
      shrd eax, edx, 8
      add eax, [setup.tan2Tilt1]
      mov u2T2T1, eax

      mov eax, baseTopLeft
      mov frameSpotTopLeft, eax
      sub eax, 3
      mov baseTopLeft, eax
      mov eax, baseTopRight
      mov frameSpotTopRight, eax
      add eax, 3
      mov baseTopRight, eax
      mov eax, baseBottomLeft
      mov frameSpotBottomLeft, eax
      sub eax, 3
      mov baseBottomLeft, eax
      mov eax, baseBottomRight
      mov frameSpotBottomRight, eax
      add eax, 3
      mov baseBottomRight, eax
    }

    fixed v2 = 0;
    fixed nDeltaV2 = setup->twoDeltaV2 >> 1;
    fixed tiltedVTop = setup->tanTilt + (setup->tiltedDeltaV >> 1);
    fixed tiltedVBottom = setup->tanTilt - (setup->tiltedDeltaV >> 1);

    for (register int y = setup->scanLines / 2; y; y--)
    {
      fixed oneRho;
      //rho = sqrt(setup->v * setup->v + u2T2T1);
      asm
      {
        mov eax, v2
        add eax, u2T2T1

        xor edx, edx  //sqrt start
        mov ebx, 40000000H
      }
      sqrt_loop:
      asm
      {
        mov ecx, edx
        shr edx, 1
        or ecx, ebx
        cmp eax, ecx
        jb short sqrt_nobit
        sub eax, ecx
        or edx, ebx
      }
      sqrt_nobit:
      asm
      {
        shr ebx, 2
        or ebx, ebx
        jnz sqrt_loop
        shl edx, 12   //sqrt end.  edx = rho

        mov ebx, edx
        xor eax, eax
        mov edx, 00010000H
        div ebx
        mov oneRho, eax
      }

      //TOP
      unsigned long index;
      //phi = asin((setup->v * setup->cosTilt + setup->tanTilt) / rho);
      asm
      {
        imul tiltedVTop
        shrd eax, edx, 24
        add eax, 00010000H // Construct index = sin(phi). Make positive.
        shr eax, 3         // right 1 (scale to 0-1) + right 4 (scale) - left 2 (sizeof(long))
        and eax, 00003FFCH // Restrict to table (should have no effect)
        mov index, eax     // serves for secant and tiltBytes tables
      }

      unsigned long center;
      //RGB* panoramaSpot = setup->panorama + (unsigned long)((setup->maxTilt - phi) * setup->tiltScale);
      asm
      {
        add eax, [setup.tiltBytes]
        mov eax, [eax]
        add eax, [setup.panoramaSpot] //Could be done ahead (faster if > TRIG_ENTRIES pels in image)
        mov center, eax  // center + tilt
      }

      unsigned long pan;
      //theta = asin(setup->u / rho / cos(phi));
      asm
      {
        mov eax, u
        imul oneRho
        shrd eax, edx, 24
        mov ebx, index
        add ebx, [setup.secant]
        mov ebx, [ebx]
        imul ebx
        shrd eax, edx, 16
        add eax, 00010000H // Construct index = sin(theta). Make positive.
        shr eax, 3         // right 1 (scale to 0-1) + right 4 (scale) - left 2 (sizeof(long))
        and eax, 00003FFCH // Restrict to table (should have no effect)
        add eax, [setup.panBytes]
        mov eax, [eax]
        mov pan, eax // pan
      }

      //Right side
      asm
      {
        add eax, center // center + tilt + pan
        cmp eax, [setup.endPanSpot]
        jb short topRightInRange
        sub eax, [setup.totalBytes]
      }
      topRightInRange:
      asm
      {
        mov edx, frameSpotTopRight
        mov eax, [eax]
        mov [edx], ax
        shr eax, 16
        mov [edx+2], al
        add edx, [setup.scanLineBytes]
        mov frameSpotTopRight, edx
      }

      //Left side
      asm
      {
        mov eax, center // center + tilt
        sub eax, pan // -pan
        sub eax, [setup.panLineBytes] // Additional 1-line offset on left to avoid centerline pairing
        cmp eax, [setup.startPanSpot]
        jge short topLeftInRange
        add eax, [setup.totalBytes]
      }
      topLeftInRange:
      asm
      {
        mov edx, frameSpotTopLeft
        mov eax, [eax]
        mov [edx], ax
        shr eax, 16
        mov [edx+2], al
        add edx, [setup.scanLineBytes]
        mov frameSpotTopLeft, edx
      }

      //BOTTOM
      asm
      {
        mov eax, oneRho
        imul tiltedVBottom
        shrd eax, edx, 24
        add eax, 00010000H
        shr eax, 3
        and eax, 00003FFCH
        mov index, eax
      }

      asm
      {
        add eax, [setup.tiltBytes]
        mov eax, [eax]
        add eax, [setup.panoramaSpot]
        mov center, eax
      }

      asm
      {
        mov eax, u
        imul oneRho
        shrd eax, edx, 24
        mov ebx, index
        add ebx, [setup.secant]
        mov ebx, [ebx]
        imul ebx
        shrd eax, edx, 16
        add eax, 00010000H
        shr eax, 3
        and eax, 00003FFCH
        add eax, [setup.panBytes]
        mov eax, [eax]
        mov pan, eax
      }

      asm
      {
        add eax, center
        cmp eax, [setup.endPanSpot]
        jb short bottomRightInRange
        sub eax, [setup.totalBytes]
      }
      bottomRightInRange:
      asm
      {
        mov edx, frameSpotBottomRight
        mov eax, [eax]
        mov [edx], ax
        shr eax, 16
        mov [edx+2], al
        sub edx, [setup.scanLineBytes]
        mov frameSpotBottomRight, edx
      }

      asm
      {
        mov eax, center
        sub eax, pan
        sub eax, [setup.panLineBytes]
        cmp eax, [setup.startPanSpot]
        jge short bottomLeftInRange
        add eax, [setup.totalBytes]
      }
      bottomLeftInRange:
      asm
      {
        mov edx, frameSpotBottomLeft
        mov eax, [eax]
        mov [edx], ax
        shr eax, 16
        mov [edx+2], al
        sub edx, [setup.scanLineBytes]
        mov frameSpotBottomLeft, edx
      }


      v2 += nDeltaV2;
      nDeltaV2 += setup->twoDeltaV2;

      tiltedVTop += setup->tiltedDeltaV;
      tiltedVBottom -= setup->tiltedDeltaV;

    }
    u += setup->deltaU;
  }
}

