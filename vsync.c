#include <GL/glxew.h>

#include "vsync.h"

void vsyncInitialize(struct VSyncController *controller)
{
  controller->isAvailable = false;

  controller->dpy = glXGetCurrentDisplay();
  controller->drawable = glXGetCurrentDrawable();

  controller->isAvailable = GLXEW_EXT_swap_control && controller->drawable;
}

bool vsyncIsAvailable(struct VSyncController *controller)
{
  return controller->isAvailable;
}

bool vsyncIsEnabled(struct VSyncController *controller)
{
  unsigned int swapInterval;

  glXQueryDrawable(controller->dpy, controller->drawable, GLX_SWAP_INTERVAL_EXT, &swapInterval);

  switch(swapInterval) {
  case 0:  return false;
  default: return true;
  }
}

void vsyncSetEnabled(struct VSyncController *controller, bool enable)
{
  unsigned int swapInterval;

  switch (enable) {
  case false: swapInterval = 0; break;
  case true:  swapInterval = 1; break;
  }

  glXSwapIntervalEXT(controller->dpy, controller->drawable, swapInterval);
}
