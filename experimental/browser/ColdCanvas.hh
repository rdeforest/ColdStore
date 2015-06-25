#ifndef COLDCANVAS_HH
#define COLDCANVAS_HH

#include <Data.hh>
#include <Tuple.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Object.hh>
#include <Frame.hh>
#include <List.hh>

#include <gnome--/canvas.h>
#include <gnome--/canvas-group.h>
#include <gnome--/canvas-rect.h>
class ColdCanvas
  : public Gnome::Canvas
{
  Gnome::CanvasGroup m_canvasgroup;

public:
  ColdCanvas();
};

class ColdView
  : public Gnome::CanvasRect
{
public:
  Slot data;
  ColdView( const Gnome::CanvasGroup &parent, const Slot &inData );
};

#endif
