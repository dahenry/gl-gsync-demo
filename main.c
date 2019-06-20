#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "gsync.h"
#include "vsync.h"

/**
 * Clock
 */

struct Clock
{
  double currentTimeSec;
  double lastTimeSec;
  double deltaSec; /* Delta Time */
};

void initializeClock(struct Clock *clock)
{
  clock->currentTimeSec = 0;
  clock->lastTimeSec = 0;
  clock->deltaSec = 0;
}

void updateClock(struct Clock *clock)
{
  clock->lastTimeSec = clock->currentTimeSec;
  clock->currentTimeSec = (double)glutGet(GLUT_ELAPSED_TIME) * 0.001f;
  clock->deltaSec = clock->currentTimeSec - clock->lastTimeSec;
}

/**
 * FrameRateController
 */

struct FrameRateController
{
  int frameRateFloor;
  int frameRateMin;
  int frameRateMax;

  double currentSimulatedFrameRate;
  double nextFrameDelaySec;
};

inline double max(double a, double b)
{
  return (a > b) ? a : b;
}

inline double min(double a, double b)
{
  return (a < b) ? a : b;
}

void initializeFrameRateController(struct FrameRateController *frameRateController)
{
  frameRateController->frameRateFloor = 10;
  frameRateController->frameRateMin = 30;
  frameRateController->frameRateMax = max(60, glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE));
}

void increaseMinFrameRate(struct FrameRateController *frameRateController, int byNrOfFrames)
{
  frameRateController->frameRateMin =
    min(frameRateController->frameRateMin + byNrOfFrames, frameRateController->frameRateMax);
}

void increaseMaxFrameRate(struct FrameRateController *frameRateController, int byNrOfFrames)
{
  frameRateController->frameRateMax += byNrOfFrames;
}

void decreaseMinFrameRate(struct FrameRateController *frameRateController, int byNrOfFrames)
{
  frameRateController->frameRateMin =
    max(frameRateController->frameRateMin - byNrOfFrames, frameRateController->frameRateFloor);
}

void decreaseMaxFrameRate(struct FrameRateController *frameRateController, int byNrOfFrames)
{
  frameRateController->frameRateMax =
    max(frameRateController->frameRateMax - byNrOfFrames, frameRateController->frameRateMin);
}

void computeNextFrameDelayMsec(struct FrameRateController *frameRateController, double currentTimeSec)
{
  const double frameRateMin = max(frameRateController->frameRateFloor, frameRateController->frameRateMin);
  const double frameRateRange = (frameRateController->frameRateMax - frameRateMin);
  const double frameRateRangeMean = (frameRateMin + frameRateController->frameRateMax) / 2.0;
  const double frameRateAmplitude = frameRateRange / 2.0;

  frameRateController->currentSimulatedFrameRate =
    frameRateRangeMean + frameRateAmplitude * sin(currentTimeSec);

  frameRateController->nextFrameDelaySec = 1.0 / frameRateController->currentSimulatedFrameRate;
}

/**
 * Application
 */

struct Application
{
  struct Clock clock;
  struct FrameRateController frameRateController;

  struct GSyncController gsyncController;
  struct VSyncController vsyncController;

  int animationSpeed;

} app;

void initializeApplication(struct Application *app)
{
  /* Application initialization */
  app->animationSpeed = 5;

  initializeClock(&app->clock);
  initializeFrameRateController(&app->frameRateController);

  vsyncInitialize(&app->vsyncController);

  /* OpenGL initialization */
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void toggleGSync(struct Application *app)
{
  gsyncSetAllowed(&app->gsyncController, !gsyncIsAllowed(&app->gsyncController));
}

void toggleVSync(struct Application *app)
{
  vsyncSetEnabled(&app->vsyncController, !vsyncIsEnabled(&app->vsyncController));
}

int printText(const char *format, ...)
{
  char buffer[1024];
  va_list arg;
  int ret;
  GLint pos[4];

  /* Format the text */
  va_start(arg, format);
    ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  /* Set raster position to force usage of last specified color */
  glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);
  glLoadIdentity();
  glRasterPos2iv(pos);

  /* Print it */
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)buffer);

  glPopMatrix();

  return ret;
}

void printStatus(const char *label, bool available, bool enabled)
{
  printText(label);

  glPushAttrib(GL_CURRENT_BIT);

  if (available) {
    glColor3f(1.0, 1.0, 0.0);
    printText("%s", enabled ? "ON" : "OFF");
  }
  else {
    glColor3f(1.0, 0.0, 0.0);
    printText("N/A");
  }

  glPopAttrib();

  /* Return to the begining of line */
  int width  = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)label);
  int height = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
  glBitmap(0, 0, 0, 0, -width, -height, NULL);
}

float computeVerticalBarXPosition()
{
  static float translation = 0.0;

  const int width = glutGet(GLUT_WINDOW_WIDTH);
  const float speedPixelPerSec = width / (float)app.animationSpeed;

  translation += speedPixelPerSec * app.clock.deltaSec;

  if (translation >= width)
      translation -= width;

  return translation;
}

void drawScene()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor3f(0.9f, 0.9f, 0.9f);

  /* Draw a vertical bar of 5% screen width */
  glTranslatef(computeVerticalBarXPosition(), 0.0f, 0.0f);
  glRecti(0, 0, glutGet(GLUT_WINDOW_WIDTH) * 0.05, glutGet(GLUT_WINDOW_HEIGHT));
}

void drawGauge(int heightPx, int gradMin, int gradMax, int gradStep, double currentValue)
{
  const int width = 15;            /* px */
  const int gradLabelYOffset = -6; /* px */

  if (gradMax == gradMin) {
    gradMin = gradMax - 10;
    gradMax = gradMax + 10;
  }

  const float gradUnitPx = heightPx / (float)(gradMax - gradMin);
  const int currentPosPx = (currentValue - gradMin) * gradUnitPx;

  glColor3f(0.0, 1.0, 0.0);

  glPushMatrix();
  glTranslated(50, 0, 0);

  glBegin(GL_LINES);

  /* Draw vertical axis */
  glVertex2i(0, 0);
  glVertex2i(0, heightPx);

  /* Draw horizontal graduations */
  for (int grad = 0; grad <= gradMax - gradMin; grad += gradStep)
   {
     int y = grad * gradUnitPx;
     glVertex2i(0, y);
     glVertex2i(width, y);
   }

  glEnd();

  /* Draw graduation labels */
  for (int grad = gradMin; grad <= gradMax; grad += gradStep)
    {
      int y = (grad - gradMin) * gradUnitPx;
      glRasterPos2i(width + 5, y + gradLabelYOffset);
      printText("%i", grad);
    }

  glPopMatrix();

  /* Draw current value */
  glPushMatrix();
  glTranslated(0, currentPosPx, 0);

  glRasterPos2i(0, gradLabelYOffset);
  printText("%.0f", currentValue);

  glTranslated(40, 0, 0);

  glBegin(GL_TRIANGLES);
  glVertex2i(10, 0);
  glVertex2i(0, -5);
  glVertex2i(0, 5);
  glEnd();

  glPopMatrix();
}

void drawHUDSeparator()
{
  glColor3f(0.0, 1.0, 0.0);

  glBegin(GL_LINES);
  glVertex2i(0, 0);
  glVertex2i(150, 0);
  glEnd();
}

void drawHUD()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslated(50, 0, 0);

  glTranslated(0, glutGet(GLUT_WINDOW_HEIGHT) * 0.1, 0);

  glColor3f(0.0, 1.0, 0.0);

  glRasterPos2i(0, 120);
  printStatus("[V] V-SYNC: ", vsyncIsAvailable(&app.vsyncController), vsyncIsEnabled(&app.vsyncController));
  printStatus("[G] G-SYNC: ", gsyncIsAvailable(&app.gsyncController), gsyncIsAllowed(&app.gsyncController));
  printText("\n");
  printText("[UP] / [DOWN] Max frame rate: %i\n", app.frameRateController.frameRateMax);
  printText("[PGUP] / [PGDOWN] Min frame rate: %i\n", app.frameRateController.frameRateMin);
  printText("[Q] / [ESC] Quit");

  glTranslated(0, 180, 0);

  drawHUDSeparator();

  glTranslated(0, 60, 0);

  drawGauge(glutGet(GLUT_WINDOW_HEIGHT) * 0.8 - 240,
	    app.frameRateController.frameRateMin,
	    app.frameRateController.frameRateMax,
	    10,
	    app.frameRateController.currentSimulatedFrameRate);
}

/**
 * GLUT's reshape callback function.
 * Update the viewport and the projection matrix.
 */
void reshape(int width, int height)
{
  if (height == 0) {
    height = 1;
  }

  glViewport(0, 0, (GLsizei)width, (GLsizei)height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, 0, height, -1.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glutPostRedisplay();
}

/**
 * GLUT's timer callback function.
 * Force redraw of the main OpenGL scene.
 */
void timerCallBack(int value)
{
  updateClock(&app.clock);
  computeNextFrameDelayMsec(&app.frameRateController, app.clock.currentTimeSec);

  /* Register next redraw */
  glutTimerFunc(app.frameRateController.nextFrameDelaySec * 1000, timerCallBack, value);

  /* Redraw scene */
  glutPostRedisplay();
}

/**
 * GLUT's display callback function.
 * Render the main OpenGL scene.
 */
void display()
{
  glClear(GL_COLOR_BUFFER_BIT);

  drawScene();
  drawHUD();

  glutSwapBuffers();
}

/**
 * GLUT's Key press callback function.
 * Called when user press a key.
 */
void keyPress(unsigned char key, int x, int y)
{
  switch (key)
    {
    case 27: /* Escape */
    case 'q': glutLeaveMainLoop(); break;
    case 'v': toggleVSync(&app); break;
    case 'g': toggleGSync(&app); break;
    }
}

/**
 * GLUT's Key press callback function.
 * Called when user press a special key.
 */
void specialKeyPress(int key, int x, int y)
{
  switch (key) {
  case GLUT_KEY_UP:        increaseMaxFrameRate(&app.frameRateController, 10); break;
  case GLUT_KEY_DOWN:      decreaseMaxFrameRate(&app.frameRateController, 10); break;
  case GLUT_KEY_PAGE_UP:   increaseMinFrameRate(&app.frameRateController, 10); break;
  case GLUT_KEY_PAGE_DOWN: decreaseMinFrameRate(&app.frameRateController, 10); break;
  }
}

int main(int argc, char *argv[])
{
  gsyncInitialize(&app.gsyncController);

  /* Force G-SYNC Visual Indicator
     For an unknown reason, we must do it twice to make it work...
     (the second call enables the first value) */
  gsyncShowVisualIndicator(&app.gsyncController, true);
  gsyncShowVisualIndicator(&app.gsyncController, true);

  /* Initialize GLUT */
  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  /* Create an OpenGL window */
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutEnterGameMode();

  glewInit();

  /* Initialize application */
  initializeApplication(&app);

  /* Setup GLUT's callback functions */
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyPress);
  glutSpecialFunc(specialKeyPress);

  glutTimerFunc(0, timerCallBack, 0);

  /* Enter the main loop */
  glutMainLoop();

  glutExit();

  gsyncFinalize(&app.gsyncController);

  return 0;
}
