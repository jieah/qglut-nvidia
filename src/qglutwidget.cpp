
#include <QtGui/QAction>
#include <QtGui/QMenuBar>
#include <QtGui/QApplication>

#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>

#include <GL/glut.h>

#include "qglutwidget.h"

/*
Assuming you are developing for Unix, you can use standard Unix file
descriptor tricks to make this happen. The basic idea is that you use
the pipe(), dup(), and dup2() functions to replace stdout and stderr.
Check their man pages (in section 2 of the manual) for more info. A
simple example:

#include <assert.h>
#include <unistd.h>

static int oldstdout;
static int oldstderr;
static int outpipe[2];
static int errpipe[2];

void funWithRedirection() {
	int result;

	result = oldstdout = dup(1);
	assert(result!=-1);
	result = oldstderr = dup(2);
	assert(result!=-1);
	result = pipe(outpipe);
	assert(result==0);
	result = pipe(errpipe);
	assert(result==0);
	result = dup2(outpipe[1], 1);
	assert(result!=-1);
	result = dup2(errpipe[1], 2);
	assert(result!=-1);
}

int getOrigOut() { return oldstderr; }
int getOrigErr() { return oldstderr; }
int getOutPipe() { return errpipe[0]; }
int getErrPipe() { return errpipe[0]; }

Now you use getOutPipe() and getErrPipe() to read from what was written
to the pipes. You can wrap a QSocket object around each file descriptor
and it will produce a readyRead() signal when data is available so you
can copy it into your text area. Note that this may work better if you
use socketpair() in place of pipe().

And for win32 see:

http://dslweb.nwnexus.com/~ast/dload/guicon.htm

*/

QGlutMainWindow::QGlutMainWindow(const QGLFormat & format)
{
	QGlutWidget * widget = new QGlutWidget(format, this);
	setCentralWidget(widget);
	widget->setFocus();
}

QGlutMainWindow::~QGlutMainWindow()
{
}

QGlutWidget * QGlutMainWindow::glutWidget()
{
	return (QGlutWidget *)centralWidget();
}

void QGlutMainWindow::setSize(int w, int h)
{
	// This only works right if QMainWindow::show
	// has already been called to establish the
	// default size of the central widget

	show();

	QSize s(w,h);
	s += size();
	s -= centralWidget()->size();
	resize(s);
}

void QGlutMainWindow::toggleFullScreen(bool checked)
{
	if (checked)
	{
		setWindowState(windowState() | Qt::WindowFullScreen);
		menuBar()->hide();
	}
	else
	{
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		if (showMenu)
		{
			menuBar()->show();
		}
	}
}

void QGlutMainWindow::toggleShowMenu(bool checked)
{
	showMenu = checked;
	if (showMenu) {
		menuBar()->show();
	}
	else {
		menuBar()->hide();
	}
}


QGlutWidget::QGlutWidget(const QGLFormat & format, QWidget * parent) : QGLWidget(format, parent),
	displayFunc(NULL),
	overlayDisplayFunc(NULL),
	reshapeFunc(NULL),
	keyboardFunc(NULL),
	keyboardUpFunc(NULL),
	mouseFunc(NULL),
	motionFunc(NULL),
	passiveMotionFunc(NULL),
	visibilityFunc(NULL),
	statusFunc(NULL),
	entryFunc(NULL),
	specialFunc(NULL),
	specialUpFunc(NULL),
	closeFunc(NULL),
	m_ignoreKeyRepeat(false)
{
	setAutoBufferSwap(false);
	setFocusPolicy(Qt::ClickFocus);

	setMinimumSize(128, 128);
}

QGlutWidget::~QGlutWidget()
{
}

void QGlutWidget::initializeGL()
{
	qglClearColor(palette().window().color());
}

void QGlutWidget::paintGL()
{
	if (displayFunc != NULL) {
		displayFunc();
	}
	else {
		glClear(GL_COLOR_BUFFER_BIT);
		swapBuffers();
	}
}

void QGlutWidget::resizeGL(int width, int height)
{
	if (reshapeFunc != NULL) {
		reshapeFunc(width, height);
	}
	else {
		glScissor(0, 0, width, height);
		glViewport(0, 0, width, height);
	}
}

void QGlutWidget::keyPressEvent(QKeyEvent * event)
{
	if (m_ignoreKeyRepeat && event->isAutoRepeat()) {
		return;
	}

	QPoint p = this->mapFromGlobal(QCursor::pos());
	int key = event->key();

	if (keyboardFunc != NULL) {
		if (key >= Qt::Key_A && key <= Qt::Key_Z) keyboardFunc(key - Qt::Key_A + 'a', p.x(), p.y());
		if (key == Qt::Key_Escape) keyboardFunc(27, p.x(), p.y());
	//	if (key == Qt::Key_Tab) keyboardFunc(, p.x(), p.y());
	//	if (key == Qt::Key_Backtab) keyboardFunc(, p.x(), p.y());
		if (key == Qt::Key_Return) keyboardFunc('\n', p.x(), p.y());
		if (key == Qt::Key_Enter) keyboardFunc('\n', p.x(), p.y());
	}
	if (specialFunc != NULL) {
		if (key >= Qt::Key_F1 && key <= Qt::Key_F12) specialFunc(key - Qt::Key_F1 + GLUT_KEY_F1, p.x(), p.y());

		if (key == Qt::Key_Left) specialFunc(GLUT_KEY_LEFT, p.x(), p.y());
		if (key == Qt::Key_Up) specialFunc(GLUT_KEY_UP, p.x(), p.y());
		if (key == Qt::Key_Right) specialFunc(GLUT_KEY_RIGHT, p.x(), p.y());
		if (key == Qt::Key_Down) specialFunc(GLUT_KEY_DOWN, p.x(), p.y());

		if (key == Qt::Key_PageUp) specialFunc(GLUT_KEY_PAGE_UP, p.x(), p.y());
		if (key == Qt::Key_PageDown) specialFunc(GLUT_KEY_PAGE_DOWN, p.x(), p.y());

		if (key == Qt::Key_Home) specialFunc(GLUT_KEY_HOME, p.x(), p.y());
		if (key == Qt::Key_End) specialFunc(GLUT_KEY_END, p.x(), p.y());
		if (key == Qt::Key_Insert) specialFunc(GLUT_KEY_INSERT, p.x(), p.y());
	}
}

void QGlutWidget::keyReleaseEvent(QKeyEvent * event)
{
	QPoint p = mapFromGlobal(QCursor::pos());
	int key = event->key();

	if (keyboardUpFunc != NULL) {
		if (key >= Qt::Key_A && key <= Qt::Key_Z) keyboardUpFunc(key - Qt::Key_A + 'a', p.x(), p.y());
		if (key == Qt::Key_Escape) keyboardUpFunc(27, p.x(), p.y());
	//	if (key == Qt::Key_Tab) keyboardUpFunc(, p.x(), p.y());
	//	if (key == Qt::Key_Backtab) keyboardUpFunc(, p.x(), p.y());
		if (key == Qt::Key_Return) keyboardUpFunc('\n', p.x(), p.y());
		if (key == Qt::Key_Enter) keyboardUpFunc('\n', p.x(), p.y());
	}
	if (specialUpFunc != NULL) {
	}
}


void QGlutWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() != 0) {
		if (motionFunc != NULL) {
			motionFunc(event->x(), event->y());
		}
	}
	else {
		if (passiveMotionFunc != NULL) {
			passiveMotionFunc(event->x(), event->y());
		}
	}
}

void QGlutWidget::mousePressEvent(QMouseEvent * event)
{
	if (mouseFunc != NULL) {
		int button = -1;
		if (event->button() == Qt::LeftButton) button = GLUT_LEFT_BUTTON;
		else if (event->button() == Qt::RightButton) button = GLUT_RIGHT_BUTTON;
		else if (event->button() == Qt::MidButton) button = GLUT_MIDDLE_BUTTON;

		if (button != -1) {
			mouseFunc(button, GLUT_DOWN, event->x(), event->y());
		}
	}
}

void QGlutWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (mouseFunc != NULL) {
		int button = -1;
		if (event->button() == Qt::LeftButton) button = GLUT_LEFT_BUTTON;
		else if (event->button() == Qt::RightButton) button = GLUT_RIGHT_BUTTON;
		else if (event->button() == Qt::MidButton) button = GLUT_MIDDLE_BUTTON;

		if (button != -1) {
			mouseFunc(button, GLUT_UP, event->x(), event->y());
		}
	}
}

#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif

void QGlutWidget::wheelEvent(QWheelEvent * event)
{
	if (mouseFunc != NULL) {
		if (event->orientation() == Qt::Vertical) {
			const int delta = event->delta();
			if (delta > 0) mouseFunc(GLUT_WHEEL_UP, GLUT_UP, event->x(), event->y());
			else if (delta < 0) if (delta > 0) mouseFunc(GLUT_WHEEL_DOWN, GLUT_UP, event->x(), event->y());
		}
	}
}

void QGlutWidget::closeEvent(QCloseEvent * event)
{
	Q_UNUSED(event);

	if (closeFunc != NULL) {
		closeFunc();
	}
}

void QGlutWidget::enterEvent(QEvent * event)
{
	Q_UNUSED(event);

	if (entryFunc != NULL) {
		entryFunc(GLUT_ENTERED);
	}
}
void QGlutWidget::leaveEvent(QEvent * event)
{
	Q_UNUSED(event);

	if (entryFunc != NULL) {
		entryFunc(GLUT_LEFT);
	}
}

void QGlutWidget::hideEvent(QHideEvent * event)
{
	Q_UNUSED(event);

	if (statusFunc != NULL) {
		statusFunc(GLUT_HIDDEN);
	}
	else if (visibilityFunc != NULL) {
		visibilityFunc(GLUT_NOT_VISIBLE);
	}
}
void QGlutWidget::showEvent(QShowEvent * event)
{
	Q_UNUSED(event);

	if (statusFunc != NULL) {
		// @@ Determine if the window is partially covered or not.
		statusFunc(GLUT_PARTIALLY_RETAINED);
	}
	else if (visibilityFunc != NULL) {
		visibilityFunc(GLUT_NOT_VISIBLE);
	}
}


void QGlutWidget::setDisplayFunc(void (*func)(void))
{
	displayFunc = func;
}

void QGlutWidget::setOverlayDisplayFunc(void (*func)(void))
{
	overlayDisplayFunc = func;
}

void QGlutWidget::setReshapeFunc(void (*func)(int width, int height))
{
	reshapeFunc = func;
}

void QGlutWidget::setKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	keyboardFunc = func;
}

void QGlutWidget::setKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	keyboardUpFunc = func;
}

void QGlutWidget::setMouseFunc(void (*func)(int button, int state, int x, int y))
{
	mouseFunc = func;
}

void QGlutWidget::setMotionFunc(void (*func)(int x, int y))
{
	motionFunc = func;
}

void QGlutWidget::setPassiveMotionFunc(void (*func)(int x, int y))
{
	passiveMotionFunc = func;
	setMouseTracking(passiveMotionFunc != NULL);
}

void QGlutWidget::setVisibilityFunc(void (*func)(int state))
{
	visibilityFunc = func;
}

void QGlutWidget::setStatusFunc(void (*func)(int state))
{
	statusFunc = func;
}

void QGlutWidget::setEntryFunc(void (*func)(int state))
{
	entryFunc = func;
}

void QGlutWidget::setSpecialFunc(void (*func)(int key, int x, int y))
{
	specialFunc = func;
}

void QGlutWidget::setSpecialUpFunc(void (*func)(int key, int x, int y))
{
	specialUpFunc = func;
}

void QGlutWidget::setCloseFunc(void (*func)(void))
{
	closeFunc = func;
}

void QGlutWidget::ignoreKeyRepeat(bool ignore)
{
	m_ignoreKeyRepeat = ignore;
}
