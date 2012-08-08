typedef long fixed;

#define float2fixed(x) ((fixed)((x) * (1 << 16)))
#define float2fixed824(x) ((fixed)((x) * (1 << 24)))
#define fixed2float(x) (((float)(x)) / (1 << 16))
#define TRIG_ENTRIES 4096

struct RendererSetup
{
  // Panorama attributes
  void *panoramaSpot;
  long startPanSpot, endPanSpot;
  unsigned long totalBytes, panLineBytes;
  fixed tanTilt, tan2Tilt1;

  // Frame attributes
  void *frameBuffer;
  unsigned long scanLines1, scanLineBytes, width1;

  // Calculation variables
  fixed deltaU, tiltedDeltaV;
  fixed twoDeltaV2; //8.24 format

  // Last-render cache
  unsigned long *lastRender;
  unsigned long lastRenderLineBytes;

  //Trig tables
  fixed *secant;
  unsigned long *tiltBytes, *panBytes;
};

