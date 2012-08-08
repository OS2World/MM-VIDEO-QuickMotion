#ifndef QUIKFLIK_HPP
#define QUIKFLIK_HPP

#include "quikflik.h"

#include <iwindow.hpp>
#include <iobjwin.hpp>
#include <iprofile.hpp>
#include <ifiledlg.hpp>
#include <iksset.h>
#include <qmmoov.h> //Help template compile?

class IResourceLibrary;
class IThread;
class QFPlayer;
class QFController;
class MovieAtom;
class PropertiesDialog;

struct ImageTypeEntry
{
  IString imageType;
  FOURCC ioProc;
};
IString const& key(ImageTypeEntry const& element);

class QuickFlick : public IVBase
{
public:

  // QuickFlick environment management
  static void              _Export initialize();
  static void              _Export shutdown();

  // Static properties
  static IResourceLibrary& _Export resourceLibrary();
  static IObjectWindow*    _Export serviceThreadControlWindow();
  static void              _Export showPropertiesDialog(IWindow* owner);
  static Boolean           _Export cueDuringPrepare();
  static long              _Export nonEmbeddedDisplay();

  static void              _Export setPath(char* path);
  static IString           _Export path();

  // Constructor
  _Export QuickFlick();

  // Instance properties
  IObjectWindow&          _Export mainThreadWindow();
  Boolean                 _Export disabled();

  void                    _Export createController(float armPercent = .0);
  void                    _Export deleteController();
  QFController*           _Export controller();

  void                    _Export setScale(float scale);
  float                   _Export scale();

  void                    _Export setAutoplay(Boolean autoplay = true);
  Boolean                 _Export willAutoplay();

  void                    _Export setLoop(Boolean loop = true);
  Boolean                 _Export willLoop();

  void                    _Export setSetMovieWindowHandleAddress(void (*setMovieWindowHandle)(IWindowHandle movieWindowHandle));
  void                    _Export (*setMovieWindowHandleAddress())(IWindowHandle);

  void                    _Export setWindow(IWindowHandle windowHandle);
  void                    _Export setMovie(IString fileName);
  virtual void            _Export playerCreated();
  IString                 _Export fileName();
  MovieAtom*              _Export movieAtom();
  HMMIO                   _Export movieFile();
  ISize                   _Export movieSize();
  IWindow*                _Export window();
  IWindowHandle           _Export logoWindowHandle();

  Boolean                 _Export autoStartPointReached();
  Boolean                 _Export streamDone();
  void                    _Export setStreamStopped(Boolean streamStopped = true);
  Boolean                 _Export streamStopped();

  IString                 _Export showSaveAsDialog(IWindow *owner);
  IString                 _Export showCaptureFrameDialog(IWindow *owner, FOURCC& ioProc);

  virtual IString         _Export hotSpotURL(long hotSpotID);
  virtual void            _Export linkTo(IString url);
  virtual void            _Export status(IString text);

  // Destructor
  virtual                 _Export ~QuickFlick();

protected:

  QFPlayer*               _Export player();
  void                    _Export setStreamDone(Boolean streamDone = true);

private:

  // QuickFlick member functions
  void getHeader();

  static IResourceLibrary     *resourceLibrary_p;
  static IObjectWindow        *serviceThreadControlWindow_p;
  static IProfile             profile;
  static IString              saveAsPath_p;
  static long                 nonEmbeddedDisplay_p;
  static Boolean              cueDuringPrepare_p;
  static IKeySortedSet<ImageTypeEntry, IString> imageTypes;

  QFPlayer                          *player_p;
  QFController                      *controller_p;
  MovieAtom                         *movieAtom_p;
  HMMIO                             movieFile_p;

  IObjectWindow                     mainThreadWindow_p;
  IThread                           *getHeaderThread;
  PULONG                            currentSize_p;
  Boolean                           disabled_p, streamStopped_p, streamDone_p;
  IString                           fileName_p, newFileName, saveAsFileName;
  Boolean                           loop_p, autoplay_p;
  float                             scale_p;
  IWindow                           *window_p;
  Boolean                           createdWin;
  void                              (*setMovieWindowHandle_p)(IWindowHandle movieWindowHandle);

  IFileDialog                       *saveAsDialog;
  IFileDialog::Settings             saveAsDialogSettings, captureFrameSettings;
};

#endif

