#pragma inline

#include "..\qfwpanof.hpp"

extern "C" __syscall void renderFrame(RendererSetup* setup)
{
  fixed u = setup->scanLineBytes - setup->scanLineBytes; //Trick stupid compiler

  void *fBaseTopLeft, *fBaseBottomLeft, *fBaseTopRight, *fBaseBottomRight;
  //Initial values seem strange because they will be incremented right away
  //(topright), (topleft)
  //(bottomright), (bottomleft)
  asm
  {
    mov eax, [setup.width1]
    mov ebx, eax
    and ebx, 00000001H
    sub ebx, 2
    mov ecx, ebx //triple 
    shl ecx, 1   //
    add ebx, ecx //ebx=-3 (even width) or -6 (odd width)
    shr eax, 1   //start in middle (halfway)
    mov ecx, eax //triple
    shl ecx, 1   //
    add eax, ecx //
    add eax, [setup.frameBuffer]
    sub eax, [setup.scanLineBytes]
    mov fBaseBottomLeft, eax
    add eax, ebx
    mov fBaseBottomRight, eax
    mov ecx, [setup.scanLines1]
    test ecx, 00000001H
    jz short fOddPels
    add eax, [setup.scanLineBytes]
  }
  fOddPels:
  asm
  {
    mov fBaseTopRight, eax
    sub eax, ebx
    mov fBaseTopLeft, eax
  }

  //Same four pointers for last-rendered offsets, but size is 32 bits instead of 24
  void *rBaseTopLeft, *rBaseBottomLeft, *rBaseTopRight, *rBaseBottomRight;
  asm
  {
    mov eax, [setup.width1]
    mov ebx, eax
    and ebx, 00000001H
    sub ebx, 2
    shl ebx, 2   //ebx=-4 (even width) or -8 (odd width)
    shr eax, 1   //start in middle (halfway)
    shl eax, 2   //quadruple
    add eax, [setup.lastRender]
    sub eax, [setup.lastRenderLineBytes]
    mov rBaseBottomLeft, eax
    add eax, ebx
    mov rBaseBottomRight, eax
    mov ecx, [setup.scanLines1]
    test ecx, 00000001H
    jz short rOddPels
    add eax, [setup.lastRenderLineBytes]
  }
  rOddPels:
  asm
  {
    mov rBaseTopRight, eax
    sub eax, ebx
    mov rBaseTopLeft, eax
  }


  mainRoutine:

  for (int x = setup->width1 / 2; x; x--)
  {
    fixed u2T2T1;
    // u2T2T1 = setup->u * setup->u + setup->tan2Tilt1;
    asm
    {
      mov eax, u
      imul dword ptr u
      shrd eax, edx, 8
      add eax, [setup.tan2Tilt1]
      mov u2T2T1, eax
    }

    void *frameSpotTopLeft, *frameSpotTopRight, *frameSpotBottomLeft, *frameSpotBottomRight;
    asm
    {
      mov eax, fBaseTopLeft
      mov ebx, fBaseTopRight
      sub eax, 3
      add ebx, 3
      mov frameSpotTopLeft, eax
      mov frameSpotTopRight, ebx
      mov fBaseTopLeft, eax
      mov fBaseTopRight, ebx
      mov eax, fBaseBottomLeft
      mov ebx, fBaseBottomRight
      sub eax, 3
      add ebx, 3
      mov frameSpotBottomLeft, eax
      mov frameSpotBottomRight, ebx
      mov fBaseBottomLeft, eax
      mov fBaseBottomRight, ebx
    }

    void *rSpotTopLeft, *rSpotTopRight, *rSpotBottomLeft, *rSpotBottomRight;
    asm
    {
      mov eax, rBaseTopLeft
      mov ebx, rBaseTopRight
      sub eax, 4
      add ebx, 4
      mov rSpotTopLeft, eax
      mov rSpotTopRight, ebx
      mov rBaseTopLeft, eax
      mov rBaseTopRight, ebx
      mov eax, rBaseBottomLeft
      mov ebx, rBaseBottomRight
      sub eax, 4
      add ebx, 4
      mov rSpotBottomLeft, eax
      mov rSpotBottomRight, ebx
      mov rBaseBottomLeft, eax
      mov rBaseBottomRight, ebx
    }

    fixed v2, nDeltaV2, tiltedVTop, tiltedVBottom;
    //nDeltaV2 = setup->twoDeltaV2 >> 1;
    //tiltedVTop = setup->tanTilt + (setup->tiltedDeltaV >> 1);
    //tiltedVBottom = setup->tanTilt - (setup->tiltedDeltaV >> 1);
    asm
    {
      mov eax, [setup.twoDeltaV2]
      mov ebx, [setup.tanTilt]
      shr eax, 1
      mov ecx, ebx
      mov nDeltaV2, eax
      mov eax, [setup.tiltedDeltaV]
      mov v2, 0
      shr eax, 1
      add ebx, eax
      sub ecx, eax
      mov tiltedVTop, ebx
      mov tiltedVBottom, ecx
    }

    for (register int y = setup->scanLines1 / 2; y; y--)
    {
      fixed oneRho;
      //rho = sqrt(setup->v * setup->v + u2T2T1);
      asm
      {
        mov eax, v2
        xor edx, edx  //sqrt start
        mov ebx, 40000000H
        add eax, u2T2T1
      }
      sqrt_loop:
      asm
      {
        mov ecx, edx
        shr edx, 1
        or ecx, ebx
        cmp ecx, eax
        jae short sqrt_nobit
        or edx, ebx
        sub eax, ecx
      }
      sqrt_nobit:
      asm
      {
        shr ebx, 2
        jnz sqrt_loop
        shl edx, 12   //sqrt end.  edx = rho
        xor eax, eax
        mov ebx, edx
        mov edx, 00010000H
        div ebx
        mov oneRho, eax
      }

      //TOP
      //phi = asin((setup->v * setup->cosTilt + setup->tanTilt) / rho);
      asm
      {
        imul tiltedVTop
        shrd eax, edx, 24
        lea ebx, [eax+00010000H] // Construct index = sin(phi). Make positive.
        mov eax, u               // theta
        shr ebx, 3               // right 1 (scale to 0-1) + right 4 (scale) - left 2 (sizeof(long))
        mov ecx, [setup.secant]  // theta
        and ebx, 00003FFCH       // Restrict to table (should have no effect)
        add ecx, ebx             // serves for secant and tiltBytes tables
      }

      //RGB* panoramaSpot = setup->panorama + (unsigned long)((setup->maxTilt - phi) * setup->tiltScale);
      //theta = asin(setup->u / rho / cos(phi));
      asm
      {
        imul oneRho                      //theta
        shrd eax, edx, 24                //theta
        add ebx, [setup.tiltBytes]       //tilt
        mov ecx, [ecx]                   //theta
        mov ebx, [ebx]                   //tilt
        imul ecx                         //theta
        shrd eax, edx, 16                //theta
        shr eax, 2         // right 4 (scale) - left 2 (sizeof(long))
        and eax, 00003FFCH // Restrict to table (should have no effect)
        add eax, [setup.panBytes]
        mov eax, [eax]
        mov ecx, eax                     // pan
      }

      //Move panorama pels to window
      asm
      {
        add eax, ebx                  //R
        mov edx, rSpotTopRight        //R
        sub ebx, ecx                  //L
        mov [edx], eax                //R
        cmp eax, [setup.endPanSpot]   //R
        mov ecx, rSpotTopLeft         //L
        jb short topRightInRange      //R
        sub eax, [setup.totalBytes]   //R
      }
      topRightInRange:
      asm
      {
        add edx, [setup.lastRenderLineBytes] //R
        mov [ecx], ebx                //L
        sub ebx, [setup.panLineBytes] //L
        mov rSpotTopRight, edx        //R
        cmp ebx, [setup.startPanSpot] //L
        mov edx, frameSpotTopRight    //R
        jge short topLeftInRange      //L
        add ebx, [setup.totalBytes]   //L
      }
      topLeftInRange:
      asm
      {
        add ecx, [setup.lastRenderLineBytes] //L
        add eax, [setup.panoramaSpot] //R
        mov rSpotTopLeft, ecx         //L
        add ebx, [setup.panoramaSpot] //L
        mov ecx, frameSpotTopLeft     //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        mov [edx], ax                 //R
        mov [ecx], bx                 //L
        shr eax, 16                   //R
        shr ebx, 16                   //L
        mov [edx+2], al               //R
        mov [ecx+2], bl               //L
        mov eax, [setup.scanLineBytes]
        add edx, eax                  //R
        add ecx, eax                  //L
        mov frameSpotTopRight, edx    //R
        mov frameSpotTopLeft, ecx     //L
      }

      //BOTTOM
      //phi = asin((setup->v * setup->cosTilt + setup->tanTilt) / rho);
      asm
      {
        mov eax, oneRho
        imul tiltedVBottom
        shrd eax, edx, 24
        lea ebx, [eax+00010000H]
        mov eax, u
        shr ebx, 3
        mov ecx, [setup.secant]
        and ebx, 00003FFCH
        add ecx, ebx
      }

      asm
      {
        imul oneRho
        shrd eax, edx, 24
        add ebx, [setup.tiltBytes]
        mov ecx, [ecx]
        mov ebx, [ebx]
        imul ecx
        shrd eax, edx, 16
        shr eax, 2
        and eax, 00003FFCH
        add eax, [setup.panBytes]
        mov eax, [eax]
        mov ecx, eax
      }

      asm
      {
        add eax, ebx                  //R
        mov edx, rSpotBottomRight     //R
        sub ebx, ecx                  //L
        mov [edx], eax                //R
        cmp eax, [setup.endPanSpot]   //R
        mov ecx, rSpotBottomLeft      //L
        jb short bottomRightInRange   //R
        sub eax, [setup.totalBytes]   //R
      }
      bottomRightInRange:
      asm
      {
        sub edx, [setup.lastRenderLineBytes] //R
        mov [ecx], ebx                //L
        sub ebx, [setup.panLineBytes] //L
        mov rSpotBottomRight, edx     //R
        cmp ebx, [setup.startPanSpot] //L
        mov edx, frameSpotBottomRight //R
        jge short bottomLeftInRange   //L
        add ebx, [setup.totalBytes]   //L
      }
      bottomLeftInRange:
      asm
      {
        sub ecx, [setup.lastRenderLineBytes] //L
        add eax, [setup.panoramaSpot] //R
        mov rSpotBottomLeft, ecx      //L
        add ebx, [setup.panoramaSpot] //L
        mov ecx, frameSpotBottomLeft  //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        mov [edx], ax                 //R
        mov [ecx], bx                 //L
        shr eax, 16                   //R
        shr ebx, 16                   //L
        mov [edx+2], al               //R
        mov [ecx+2], bl               //L
        mov eax, [setup.scanLineBytes]
        sub edx, eax                  //R
        sub ecx, eax                  //L
        mov frameSpotBottomRight, edx    //R
        mov frameSpotBottomLeft, ecx     //L
      }

      //v2 += nDeltaV2;
      //nDeltaV2 += setup->twoDeltaV2;
      //tiltedVTop += setup->tiltedDeltaV;
      //tiltedVBottom -= setup->tiltedDeltaV;
      asm
      {
        mov eax, nDeltaV2
        mov ebx, [setup.tiltedDeltaV]
        add v2, eax
        add tiltedVTop, ebx
        add eax, [setup.twoDeltaV2]
        sub tiltedVBottom, ebx
        mov nDeltaV2, eax
      }

    }
    u += setup->deltaU;
  }
}


extern "C" __syscall void quickRenderFrame(RendererSetup* setup)
{
  fixed u = setup->scanLineBytes - setup->scanLineBytes; //Trick stupid compiler

  void *fBaseTopLeft, *fBaseBottomLeft, *fBaseTopRight, *fBaseBottomRight;
  //Initial values seem strange because they will be incremented right away
  //(topright), (topleft)
  //(bottomright), (bottomleft)
  asm
  {
    mov eax, [setup.width1]
    mov ebx, eax
    and ebx, 00000001H
    sub ebx, 2
    mov ecx, ebx //triple 
    shl ecx, 1   //
    add ebx, ecx //ebx=-3 (even width) or -6 (odd width)
    shr eax, 1   //start in middle (halfway)
    mov ecx, eax //triple
    shl ecx, 1   //
    add eax, ecx //
    add eax, [setup.frameBuffer]
    sub eax, [setup.scanLineBytes]
    mov fBaseBottomLeft, eax
    add eax, ebx
    mov fBaseBottomRight, eax
    mov ecx, [setup.scanLines1]
    test ecx, 00000001H
    jz short fOddPels
    add eax, [setup.scanLineBytes]
  }
  fOddPels:
  asm
  {
    mov fBaseTopRight, eax
    sub eax, ebx
    mov fBaseTopLeft, eax
  }

  //Same four pointers for last-rendered offsets, but size is 32 bits instead of 24
  void *rBaseTopLeft, *rBaseBottomLeft, *rBaseTopRight, *rBaseBottomRight;
  asm
  {
    mov eax, [setup.width1]
    mov ebx, eax
    and ebx, 00000001H
    sub ebx, 2
    shl ebx, 2   //ebx=-4 (even width) or -8 (odd width)
    shr eax, 1   //start in middle (halfway)
    shl eax, 2   //quadruple
    add eax, [setup.lastRender]
    sub eax, [setup.lastRenderLineBytes]
    mov rBaseBottomLeft, eax
    add eax, ebx
    mov rBaseBottomRight, eax
    mov ecx, [setup.scanLines1]
    test ecx, 00000001H
    jz short rOddPels
    add eax, [setup.lastRenderLineBytes]
  }
  rOddPels:
  asm
  {
    mov rBaseTopRight, eax
    sub eax, ebx
    mov rBaseTopLeft, eax
  }

  mainRoutine:

  for (int x = setup->width1 / 2; x; x--)
  {
    void *frameSpotTopLeft, *frameSpotTopRight, *frameSpotBottomLeft, *frameSpotBottomRight;
    asm
    {
      mov eax, fBaseTopLeft
      mov ebx, fBaseTopRight
      sub eax, 3
      add ebx, 3
      mov frameSpotTopLeft, eax
      mov frameSpotTopRight, ebx
      mov fBaseTopLeft, eax
      mov fBaseTopRight, ebx
      mov eax, fBaseBottomLeft
      mov ebx, fBaseBottomRight
      sub eax, 3
      add ebx, 3
      mov frameSpotBottomLeft, eax
      mov frameSpotBottomRight, ebx
      mov fBaseBottomLeft, eax
      mov fBaseBottomRight, ebx
    }

    void *rSpotTopLeft, *rSpotTopRight, *rSpotBottomLeft, *rSpotBottomRight;
    asm
    {
      mov eax, rBaseTopLeft
      mov ebx, rBaseTopRight
      sub eax, 4
      add ebx, 4
      mov rSpotTopLeft, eax
      mov rSpotTopRight, ebx
      mov rBaseTopLeft, eax
      mov rBaseTopRight, ebx
      mov eax, rBaseBottomLeft
      mov ebx, rBaseBottomRight
      sub eax, 4
      add ebx, 4
      mov rSpotBottomLeft, eax
      mov rSpotBottomRight, ebx
      mov rBaseBottomLeft, eax
      mov rBaseBottomRight, ebx
    }

    for (register int y = setup->scanLines1 / 2; y; y--)
    {
      //TOP
      asm
      {
        mov eax, rSpotTopRight        //R
        mov ebx, rSpotTopLeft         //L
        mov ecx, eax                  //R
        mov edx, ebx                  //L
        add ecx, [setup.lastRenderLineBytes] //R
        add edx, [setup.lastRenderLineBytes] //L
        mov rSpotTopRight, ecx        //R
        mov rSpotTopLeft, edx         //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        cmp eax, [setup.endPanSpot]   //R
        jb short topRightInRange      //R
        sub eax, [setup.totalBytes]   //R
      }
      topRightInRange:
      asm
      {
        add eax, [setup.panoramaSpot] //R
        sub ebx, [setup.panLineBytes] //L
        cmp ebx, [setup.startPanSpot] //L
        jge short topLeftInRange      //L
        add ebx, [setup.totalBytes]   //L
      }
      topLeftInRange:
      asm
      {
        add ebx, [setup.panoramaSpot] //L
        mov edx, frameSpotTopRight    //R
        mov ecx, frameSpotTopLeft     //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        mov [edx], ax                 //R
        mov [ecx], bx                 //L
        shr eax, 16                   //R
        shr ebx, 16                   //L
        mov [edx+2], al               //R
        mov [ecx+2], bl               //L
        mov eax, [setup.scanLineBytes]
        add edx, eax                  //R
        add ecx, eax                  //L
        mov frameSpotTopRight, edx    //R
        mov frameSpotTopLeft, ecx     //L
      }

      //BOTTOM
      asm
      {
        mov eax, rSpotBottomRight        //R
        mov ebx, rSpotBottomLeft         //L
        mov ecx, eax                  //R
        mov edx, ebx                  //L
        sub ecx, [setup.lastRenderLineBytes] //R
        sub edx, [setup.lastRenderLineBytes] //L
        mov rSpotBottomRight, ecx        //R
        mov rSpotBottomLeft, edx         //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        cmp eax, [setup.endPanSpot]   //R
        jb short bottomRightInRange      //R
        sub eax, [setup.totalBytes]   //R
      }
      bottomRightInRange:
      asm
      {
        add eax, [setup.panoramaSpot] //R
        sub ebx, [setup.panLineBytes] //L
        cmp ebx, [setup.startPanSpot] //L
        jge short bottomLeftInRange      //L
        add ebx, [setup.totalBytes]   //L
      }
      bottomLeftInRange:
      asm
      {
        add ebx, [setup.panoramaSpot] //L
        mov edx, frameSpotBottomRight    //R
        mov ecx, frameSpotBottomLeft     //L
        mov eax, [eax]                //R
        mov ebx, [ebx]                //L
        mov [edx], ax                 //R
        mov [ecx], bx                 //L
        shr eax, 16                   //R
        shr ebx, 16                   //L
        mov [edx+2], al               //R
        mov [ecx+2], bl               //L
        mov eax, [setup.scanLineBytes]
        sub edx, eax                  //R
        sub ecx, eax                  //L
        mov frameSpotBottomRight, edx    //R
        mov frameSpotBottomLeft, ecx     //L
      }
    }
  }
  u += setup->scanLineBytes; //More trickery
}

