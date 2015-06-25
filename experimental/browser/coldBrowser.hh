// generated 2001/6/8 15:49:50 EDT by ryan@nailpolish.(none)
// using glademm V0.5_11f
//
// newer (non customized) versions of this file go to coldBrowser.hh_new

// you might replace
//    class foo : public foo_glade { ... };
// by
//    typedef foo_glade foo;
// if you didn't make any modifications to the widget


#include <Data.hh>
#include <Frame.hh>

#include <gdk--/font.h>
#include <gtk--/tree.h>



#ifndef _COLDBROWSER_HH
#  define _COLDBROWSER_HH
#  include "coldBrowser_glade.hh"

class coldBrowser : public coldBrowser_glade
{   
        
  friend class coldBrowser_glade;
  Gdk_Font fixed_font;

  Slot right_objects;

public:
  coldBrowser( const Slot &object_list );

  void setNewMemberName( string i );
  Gtk::TreeItem *top_item;

  void clearTree();


  Slot selectedParent();
  Slot selectedChild();
  Slot selectedMember();
  void populateLeft( const Slot &object_list );
  void populateRight( const Slot &object_list );
  void compile_click();
  void remove_click();
  void member_click();
  void new_member_click();
  void left_panel_click();
  void right_panel_click();
  void parents_click();
  void all_click();

  void showTree( const Slot &closure );
  void inspectObject( Slot obj );
  void viewMembers( Slot obj );
  void viewSource( Slot obj, Slot method );
};
#endif
