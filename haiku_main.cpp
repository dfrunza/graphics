#include <stdlib.h>
#include <stdio.h>
#include <Application.h>
#include <Window.h>

class DrawingApp : public BApplication
{
public:
  DrawingApp(const char*);
};

DrawingApp::DrawingApp(const char* id) : BApplication(id)
{
}

int main()
{
  DrawingApp* app = new DrawingApp("application/x-vnd.dfrunza-drawing");
  BRect window_rect(50, 50, 100, 100);
  BWindow* window = new BWindow(window_rect, "Drawing", B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE);
  window->Show();
  app->Run();
  return 0;
}
