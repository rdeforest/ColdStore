#include <string>
#include <vector>
using std::vector;


#include <Data.hh>
#include <Slot.hh>
#include <Tuple.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Store.hh>
#include <String.hh>
#include <Frame.hh>
#include <Object.hh>
#include <Closure.hh>

#include "config.h"
#include "coldBrowser.hh"

namespace gtk {
#include <gtk--/label.h>
#include <gtk--/listitem.h>
}
#include <gdk--/font.h>

#include <gnome--/dialog.h>
#include <gnome--/canvas.h>


using SigC::slot;

Gtk::TreeItem *tree_recurse( Slot node, Gtk::Tree *tree ) {
  bool isBranch = false;
  if ( node->isSequence() && !(AKO((Data*)(node), String )) && node->length() ) {
    isBranch = true;
  }
  char *label = (char*)node.typeId();

  Gtk::TreeItem *item = new Gtk::TreeItem( label );
  tree->append( *item );
  if (isBranch) {
    Gtk::Tree *sub_tree = new Gtk::Tree();
    item->set_subtree( *sub_tree );
    Slot n_it = node->iterator();
    while (n_it->More()) {
      Slot sub_node = n_it->Next();
      tree_recurse( sub_node, sub_tree );
    }
  }
  item->show();
  return item;
}

Slot coldBrowser::selectedParent() {
  Gtk::List::SelectionList &dlist=parents->selection();
  Gtk::List::SelectionList::iterator iter;
  Slot oid;
  if (dlist.empty()) {
     return (Data*)NULL;
  }  
  for (iter=dlist.begin();iter!=dlist.end();++iter)
    oid = dynamic_cast<Gtk::Label*>((*iter)->get_child())->get().c_str();
  
  Slot obj = store->Names->slice(oid)[2];

  return obj;
}

Slot coldBrowser::selectedChild() {
  Gtk::List::SelectionList &dlist=children->selection();
  Gtk::List::SelectionList::iterator iter;
  Slot oid;
  if (dlist.empty()) {
     return (Data*)NULL;
  }  
  for (iter=dlist.begin();iter!=dlist.end();++iter)
    oid = dynamic_cast<Gtk::Label*>((*iter)->get_child())->get().c_str();
  
  Slot obj = store->Names->slice(oid)[2];

  return obj;
}

void coldBrowser::clearTree() {
  if (top_item) {
    nodeTree->remove_item(*top_item);
    top_item = (Gtk::TreeItem*)NULL;
  }
}

void coldBrowser::showTree( const Slot &closure ) {
  clearTree();
  Slot top = ((Closure*)(Data*)closure)->getTree();
  top_item = tree_recurse( top, nodeTree );
  show_all();
}

void coldBrowser::inspectObject( Slot member ) {
}

Slot coldBrowser::selectedMember() {
    Gtk::List::SelectionList &dlist=members->selection();
    Gtk::List::SelectionList::iterator iter;
    Slot name;

    if (dlist.empty()) {
        return (Data*)NULL;
    }
 
    for (iter=dlist.begin();iter!=dlist.end();++iter)
      name = dynamic_cast<Gtk::Label*>((*iter)->get_child())->get().c_str();

    return name;
}

void coldBrowser::viewSource(Slot obj, Slot method) {
  source->delete_text(0,-1);
  Slot value = obj->slice(method)[2];
  if ( (AKO((Data*)(value), Closure )) ){
    Closure *closure = ((Closure*)(Data*)value);
    Slot m_source = closure->getSource();
    if (m_source) {
      Slot s_it = m_source->iterator();
      while (s_it->More()) {
	Slot line = s_it->Next()->concat("\n");
	source->insert( fixed_font, (char*)line );
      }
    }
    showTree( closure );
  }
  show_all();
}

void coldBrowser::viewMembers( Slot obj ) {
  members->clear_items(0, -1);
  Slot o_it = obj->iterator();
  Gtk::ListItem *list_item;
  while (o_it->More()) {
    Slot name = o_it->Next()[1];
    list_item = manage( new Gtk::ListItem( (char*)name ) );
    members->add( *list_item );
  }
  show_all();
}

void coldBrowser::populateLeft( const Slot &object_list ) {
  parents->clear_items(0,-1) ;
  Slot object_it = object_list->iterator();
  Gtk::ListItem * list_item;

  while (object_it->More()) {
    Slot o = object_it->Next();
    try {
      list_item = manage( new Gtk::ListItem( (char*)o.id) );
      parents->add( *list_item );
    } catch (Error *e) {}
    show_all();
  }

}

void coldBrowser::populateRight( const Slot &object_list ) {
  right_objects = object_list;

  children->clear_items(0,-1) ;
  Slot object_it = object_list->iterator();
  Gtk::ListItem * list_item;

  while (object_it->More()) {
    Slot o = object_it->Next();
    try {
      list_item = manage( new Gtk::ListItem( (char*)o.id) );
      children->add( *list_item );
    } catch (Error *e) {}
    show_all();
  }

}

void coldBrowser::setNewMemberName( string i ) {
  Slot o_name = i.c_str();
  Slot so = selectedParent();
  so->insert(o_name, 1);
  viewMembers(so);
}

void coldBrowser::parents_click() {
  Frame *context = (Frame*)0;
  Slot o = selectedParent();
  Slot p = selectedChild();
  if (p)
    o = p;

  if (o) {
    Slot parent_l = ((::Object*)(Data*)o)->parents();
    if (parent_l->length()) {
      populateRight( new List(-1, (Data*)o ));
      populateLeft( parent_l );
      parents->select_item( 0 );
    }
  }
}

void coldBrowser::all_click() {
  Slot object_list = new List();
  Slot names_it = store->Names->iterator();
  while (names_it->More())
    object_list->insert(names_it->Next()[2]);

  populateLeft( object_list ); 
  populateRight( new List() );
}

void coldBrowser::remove_click() {
  Slot o = selectedParent();
  Slot m = selectedMember();

  if (o && m) {
    Slot sym = o->slice(m);
    o->del(sym);
  }
  viewMembers(o);
}

void coldBrowser::left_panel_click() {
  Slot o = selectedParent();

  if (o) {
    viewMembers( o );
    populateRight( ((::Object*)(Data*)o)->children() );
  }
    
}



void coldBrowser::right_panel_click() {
  Slot o = selectedChild();
  if (o) {
    viewMembers( o );
    Slot children = ((::Object*)(Data*)o)->children();
    if (children->length()) {
      populateLeft( right_objects );
      parents->select_item( right_objects->search(o) );
      populateRight( children );

    }
  }
}

void coldBrowser::member_click() {
  Slot so = selectedParent();
  Slot sc = selectedChild();
  if (sc)
    so = sc;

  Slot sm = selectedMember();
  if (so && sm) {
    clearTree();
    Slot mem = so->slice(sm);
    inspectObject( so->slice(sm) );
    viewSource( so, sm );
    
  }
}

void coldBrowser::new_member_click() {
  Slot so = selectedParent();
  if (so)
    Gnome::Dialogs::request( false, "Enter new member name", "", 50, slot(this, &coldBrowser::setNewMemberName) );
}

void coldBrowser::compile_click() {
  Frame *context = (Frame*)0;

  Slot so = selectedParent();
  Slot sm = selectedMember();
  if (sm && so) {
    const char *x = source->get_chars(0, -1).c_str();
    Slot line = "";
    Slot code = new List();

    for (int y=0;y<=strlen(x);y++) {
      if (x[y]!='\n')
	line = line->concat( new String( x+y, 1 ));
      else {
	code->insert(line);
	line = "";
      }
    }
    try {
      Slot fc = store->Names->slice("freon_compiler")[2];
      Slot method = fc.compile( code, so, sm );
      if (method) {
	so->insert( sm, method );
	showTree( ((Frame*)(Data*)method));
      }
    } catch (Frame *f) {
      f->upcount();
      Gnome::Dialogs::error( (char*)f->traceback() );
      f->dncount();
    }
  }
}

coldBrowser::coldBrowser( const Slot &object_list ) : coldBrowser_glade() {
  top_item = (Gtk::TreeItem*)NULL;
  fixed_font = Gdk_Font( "-*-lucida-medium-r-*-*-14-*-*-*-*-*-*-*");
  populateLeft( object_list ); 
  members->selection_changed.connect(slot(this, &coldBrowser::member_click));

  parents->selection_changed.connect(slot(this, &coldBrowser::left_panel_click));
  children->selection_changed.connect(slot(this, &coldBrowser::right_panel_click));

  newMember->clicked.connect(slot(this, &coldBrowser::new_member_click));
  compileButton->clicked.connect(slot(this, &coldBrowser::compile_click));
  removeMember->clicked.connect(slot(this, &coldBrowser::remove_click));
  showParents->clicked.connect(slot(this, &coldBrowser::parents_click));
  showAll->clicked.connect(slot(this, &coldBrowser::all_click));
}

