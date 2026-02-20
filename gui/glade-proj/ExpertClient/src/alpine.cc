// generated 2001/2/26 23:12:28 CST by root@logic.
// using glademm V0.5_11
//
// newer (non customized) versions of this file go to alpine.cc_glade

// This file is for your program, I won't touch it again!

#include <gtk--/main.h>

#include "MainWindow.hh"

int main(int argc, char **argv)
{   
   
   Gtk::Main m(&argc, &argv);

MainWindow *mainwindow = new MainWindow();
   m.run();
delete mainwindow;
   return 0;
}
