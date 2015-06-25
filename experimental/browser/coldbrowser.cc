// generated 2001/6/8 15:49:50 EDT by ryan@nailpolish.(none)
// using glademm V0.5_11f
//
// newer (non customized) versions of this file go to coldbrowser.cc_new

// This file is for your program, I won't touch it again!

#include <gtk--/main.h>

#include "coldBrowser.hh"

int main(int argc, char **argv)
{   
   
   Gtk::Main m(&argc, &argv);
manage(new class coldBrowser());
   m.run();
   return 0;
}
