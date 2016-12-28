#include "aos/vision/events/gtk_event.h"

using namespace aos::events;
using namespace aos::vision;

#include <gtk/gtk.h>

int main (int argc, char *argv[]) {
  EpollLoop loop;
  add_gtk_main(&loop, &argc, &argv);

  GtkWidget* window;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Window");

  gtk_widget_show_all(window);

  loop.Run();
}
