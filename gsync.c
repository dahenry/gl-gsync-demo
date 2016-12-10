#include <stdio.h>
#include <GL/glxew.h>
#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

#include "gsync.h"

int gsyncInitialize(struct GSyncController *controller)
{
  int event_base, error_base, value;

  controller->isAvailable = false;

  /*
   * Open a connection to the X server indicated by the DISPLAY
   * environment variable
   */

  controller->dpy = XOpenDisplay(NULL);
  if (!controller->dpy) {
    fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(NULL));
    return 0;
  }

  /*
   * Check if the NV-CONTROL X extension is present on this X server
   */

  if (!XNVCTRLQueryExtension(controller->dpy, &event_base, &error_base)) {
    fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n", XDisplayName(NULL));
    return 0;
  }

  /*
   * Check if the GSYNC attribute is present
   */

  if (!XNVCTRLQueryAttribute(controller->dpy, 0, 0, NV_CTRL_GSYNC_ALLOWED, &value)) {
    fprintf(stderr, "The NV-CONTROL GSYNC attribute is not available on '%s'.\n", XDisplayName(NULL));
    return 0;
  }

  controller->isAvailable = true;

  /*
   * Save current attributes
   */

  controller->initialGSYNCValue = gsyncIsAllowed(controller);
  controller->initialGSYNCVisualIndicatorValue = gsyncIsVisualIndicatorShown(controller);

  return 1;
}

void gsyncFinalize(struct GSyncController *controller)
{
  if (controller->isAvailable) {
    /* Restore initial attributes */
    gsyncSetAllowed(controller, controller->initialGSYNCValue);
    gsyncShowVisualIndicator(controller, controller->initialGSYNCVisualIndicatorValue);
  }

  if (controller->dpy != NULL) {
    XCloseDisplay(controller->dpy);
  }
}

bool gsyncIsAvailable(struct GSyncController *controller)
{
  return controller->isAvailable;
}

bool gsyncIsAllowed(struct GSyncController *controller)
{
  int value;

  if (controller->dpy != NULL) {
    if (XNVCTRLQueryAttribute(controller->dpy, 0, 0, NV_CTRL_GSYNC_ALLOWED, &value)) {
      switch(value) {
      case NV_CTRL_GSYNC_ALLOWED_FALSE: return false;
      case NV_CTRL_GSYNC_ALLOWED_TRUE:  return true;
      default:                          return false;
      }
    }
  }

  return false;
}

void gsyncSetAllowed(struct GSyncController *controller, bool enable)
{
  int value;

  switch (enable) {
  case false: value = NV_CTRL_GSYNC_ALLOWED_FALSE; break;
  case true:  value = NV_CTRL_GSYNC_ALLOWED_TRUE;  break;
  }

  XNVCTRLSetAttribute(controller->dpy, 0, 0, NV_CTRL_GSYNC_ALLOWED, value);
}

bool gsyncIsVisualIndicatorShown(struct GSyncController *controller)
{
  int value;

  if (controller->dpy != NULL) {
    if (XNVCTRLQueryAttribute(controller->dpy, 0, 0, NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR, &value)) {
      switch(value) {
      case NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR_FALSE: return false;
      case NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR_TRUE:  return true;
      default:                                        return false;
      }
    }
  }

  return false;
}

void gsyncShowVisualIndicator(struct GSyncController *controller, bool enable)
{
  int value;

  switch (enable) {
  case false: value = NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR_FALSE; break;
  case true:  value = NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR_TRUE;  break;
  }

  XNVCTRLSetAttribute(controller->dpy, 0, 0, NV_CTRL_SHOW_GSYNC_VISUAL_INDICATOR, value);
}
