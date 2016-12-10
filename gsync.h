#ifndef __GSYNC_H__
#define __GSYNC_H__

#include <stdbool.h>
#include <X11/Xlib.h>

struct GSyncController
{
  bool isAvailable;
  Display *dpy;

  bool initialGSYNCValue;
  bool initialGSYNCVisualIndicatorValue;
};

int gsyncInitialize(struct GSyncController *controller);
void gsyncFinalize(struct GSyncController *controller);

bool gsyncIsAvailable(struct GSyncController *controller);

bool gsyncIsAllowed(struct GSyncController *controller);
void gsyncSetAllowed(struct GSyncController *controller, bool enable);

bool gsyncIsVisualIndicatorShown(struct GSyncController *controller);
void gsyncShowVisualIndicator(struct GSyncController *controller, bool enable);

#endif /* __GSYNC_H__ */
