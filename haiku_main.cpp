#include <stdlib.h>
#include <stdio.h>
#include <Application.h>
#include <Window.h>
#include <View.h>

class DrawingApp : public BApplication
{
public:
  DrawingApp(const char* signature);
};

DrawingApp::DrawingApp(const char* signature) : BApplication(signature)
{
}

class AppWindow : public BWindow
{
public:
  AppWindow(BRect frame, const char *title, window_type type, uint32 flags);
  void MessageReceived(BMessage* msg);
};

AppWindow::AppWindow(BRect frame, const char *title, window_type type, uint32 flags) :
  BWindow(frame, title, type, flags, B_CURRENT_WORKSPACE)
{
}

void AppWindow::MessageReceived(BMessage* msg)
{
  switch (msg->what) {
    default:
      BWindow::MessageReceived(msg);
      break;
  }
}

class BitmapView : public BView
{
public:
  BitmapView(BRect frame, const char* name, uint32 resizingMode, uint32 flags);
  void Draw(BRect rect);
};

BitmapView::BitmapView(BRect frame, const char* name, uint32 resizingMode, uint32 flags)
  : BView(frame, name, resizingMode, flags)
{
}

void BitmapView::Draw(BRect rect)
{
  SetHighColor(128, 0, 0);
  FillRect(Bounds());
}

int main()
{
  DrawingApp* app = new DrawingApp("application/x-vnd.dfrunza-drawing");
  BRect window_rect(50, 50, 100, 100);
  AppWindow* window = new AppWindow(window_rect, "Drawing", B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE);
  BRect view_rect(0, 0, 100, 100);
  BitmapView* view = new BitmapView(view_rect, 0, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
  window->AddChild(view);
  window->Show();
  app->Run();
  return 0;
}
