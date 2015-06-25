#include <Data.hh>
#include <Tuple.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Object.hh>
#include <Frame.hh>

#include "ColdCanvas.hh"

#include <gnome--/canvas-rect.h>

ColdCanvas::ColdCanvas()
  : m_canvasgroup( *(root()), 0, 0 )
{
  ColdView *v = new ColdView( m_canvasgroup, new List() );

  (*v).set_x1(10).set_y1(10).
    set_x2(50).set_y2(100).set_fill_color("white");
  
}

ColdView::ColdView( const Gnome::CanvasGroup &parent, const Slot &inData )
  : Gnome::CanvasRect( parent, "width_pixels", 2,
		       "outline_color", "black",
		       "fill_color", "white",
		       NULL  ),
    data(inData)
{
  
}

