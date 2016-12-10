#ifndef __VSYNC_H__
#define __VSYNC_H__

#include <stdbool.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

struct VSyncController
{
  bool isAvailable;
  Display *dpy;
  GLXDrawable drawable;
};

void vsyncInitialize(struct VSyncController *controller);
void vsyncFinalize(struct VSyncController *controller);

bool vsyncIsAvailable(struct VSyncController *controller);

bool vsyncIsEnabled(struct VSyncController *controller);
void vsyncSetEnabled(struct VSyncController *controller, bool enable);

#endif /* __VSYNC_H__ */
