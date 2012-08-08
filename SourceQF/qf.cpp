#include "moviefrm.hpp" //MovieFrame

#include <idmhndlr.hpp> //IDMHandler
#include <iapp.hpp>     //IApplication
#include <iaccel.hpp>   //IAccelerator

#include "qf.h"

MovieFrame *movieFrame;

void main(int argc, char *argv[])
{
  try
  {
    IAccelerator accelerator(QF_ID);

    QuickFlick::initialize();
    movieFrame = new MovieFrame;
    if (argc > 1)
      movieFrame->setMovie(argv[1]);
    else
      movieFrame->showWindow();
    IApplication::current().run();

    delete movieFrame;
    QuickFlick::shutdown();
  }
  catch (IException ie)
  {
  }
}

ImageItem::ImageItem(const IDMItem::Handle& item)
         : IDMItem(item)
{
}

Boolean ImageItem::targetDrop(IDMTargetDropEvent& event)
{
  movieFrame->imageCenter = targetOperation()->position();
  movieFrame->newFileName = containerName() + sourceName();
  movieFrame->postEvent(WM_CUSTOM_SETMOVIE);
  return true;
}

// Callback function to enable drop on movie window once QF creates it.

void setMovieWindowHandle(IWindowHandle movieWindowHandle)
{
  if (movieWindowHandle)
  {
    IString unqualifiedFileName = movieFrame->quickFlick->fileName();
    unsigned long lastDelimiterIndex = unqualifiedFileName.lastIndexOf('\\');
    unqualifiedFileName = unqualifiedFileName.subString(lastDelimiterIndex + 1, unqualifiedFileName.length() - lastDelimiterIndex);
    movieFrame->title.setObjectText("QuickFlick");
    movieFrame->title.setViewText(unqualifiedFileName);
    movieFrame->addToWindowList();

    movieFrame->movieWindow = IWindow::windowWithHandle(movieWindowHandle);
    IDMHandler::enableDropOn(movieFrame->movieWindow);
    movieFrame->movieWindow->setItemProvider(movieFrame);
  }

  if (movieFrame->quickFlick->logoWindowHandle())
  {
    movieFrame->logo = IWindow::windowWithHandle(movieFrame->quickFlick->logoWindowHandle());
    IDMHandler::enableDropOn(movieFrame->logo);
    movieFrame->logo->setItemProvider(movieFrame);
  }

  if (movieWindowHandle && QuickFlick::nonEmbeddedDisplay() == QF_NONEMBED_TOFIT)
    movieFrame->reset();
  else
    movieFrame->showWindow();
}

