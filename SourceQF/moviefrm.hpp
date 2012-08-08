#define INCL_GPI
#define INCL_GPIBITMAPS
#define INCL_WIN
#define INCL_WINTRACKRECT
#define INCL_WINFRAMEMGR
#include <os2.h>

#include <iframe.hpp>   //IFrameWindow
#include <ititle.hpp>   //ITitle
#include <isysmenu.hpp> //ISystemMenu
#include <ipopmenu.hpp> //IPopUpMenu
#include <icanvas.hpp>  //ICanvas
#include <istattxt.hpp> //IStaticText
#include <ifiledlg.hpp> //IFileDialog
#include <istring.hpp>  //IString
#include <icmdhdr.hpp>  //ICommandHandler
#include <imenuhdr.hpp> //IMenuHandler
#include <idmprov.hpp>  //IDMItemProvider
#include <idmitem.hpp>  //IDMItem
#include <idmevent.hpp> //IDMEvent
#include <imoushdr.hpp> //IMouseHandler
#include <iframhdr.hpp> //IFrameHandler

#include "mfcsthlr.hpp" //MovieFrameCustomHandler
#include "quikflik.hpp" //QuickFlick

class ImageItem : public IDMItem
{
public:
  ImageItem(const IDMItem::Handle& item);
  virtual Boolean targetDrop(IDMTargetDropEvent& event);
};

class MovieFrame : private IFrameWindow,
                   private ICommandHandler, private IMenuHandler,
                   private MovieFrameCustomHandler, private IMouseHandler,
                   private IFrameHandler, private IDMItemProviderFor<ImageItem>
{
  public:
    MovieFrame();
    void setMovie(IString fileName);
    void showWindow();
    ~MovieFrame();

 protected:
   virtual Boolean track(TrackEvent& event);
   virtual Boolean minMax(MinMaxEvent& event);
   virtual Boolean setMovie(IEvent& event);
   virtual Boolean makePopUpMenu(IMenuEvent& event);
   virtual Boolean menuShowing(IMenuEvent& event, ISubmenu& submenu);
   virtual Boolean command(ICommandEvent& event);
   virtual Boolean systemCommand(ICommandEvent& event);
   virtual Boolean mouseClicked(IMouseClickEvent& event);
   virtual Boolean provideEnterSupport(IDMTargetEnterEvent& event);
   virtual Boolean provideLeaveSupport(IDMTargetLeaveEvent& event);
   virtual Boolean closed(IFrameEvent& event);

  private:
  /* Private functions */
    void moveOntoScreen(IRectangle& rect);
    void showErrorMessage();
    void showInfoMessage();
    void reset();
    Boolean validFile(IString fileName);

    QuickFlick *quickFlick;
    IWindow *movieWindow, *logo;

  /* Image attributes */
    IPoint imageCenter;
    Boolean grabbable;
    Boolean stretchable;

  /* Window attributes */
    ISize screenSize, usableScreenSize;
    ISize titleBarSize;
    ITitle title;
    IRectangle clientRestoreRect;
    ISystemMenu systemMenu;
    IPopUpMenu *popUpMenu;
    IStaticText canvas;
    IFileDialog *fileDialog_p;
    IString newFileName;

  friend ImageItem;
  friend void setMovieWindowHandle(IWindowHandle movieWindowHandle);
};

