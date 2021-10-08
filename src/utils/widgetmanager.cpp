#include "widgetmanager.h"

#include <QStyle>
#include <QApplication>
#include <QScreen>

// WindowSticker
class WindowSticker :
	public QObject
{
public:
	static WindowSticker *instance();
	static void insertWindow(QWidget *AWindow);
	static void removeWindow(QWidget *AWindow);
protected:
	bool eventFilter(QObject *AWatched, QEvent *AEvent);
private:
	WindowSticker();
	int FStickEvent;
	QPoint FStickPos;
	QWidget *FCurWindow;
};

WindowSticker::WindowSticker()
{
	FCurWindow = NULL;
	FStickEvent = QEvent::registerEventType();
}

WindowSticker *WindowSticker::instance()
{
	static WindowSticker *inst = new WindowSticker;
	return inst;
}

void WindowSticker::insertWindow(QWidget *AWindow)
{
	if (AWindow)
		AWindow->installEventFilter(instance());
}

void WindowSticker::removeWindow(QWidget *AWindow)
{
	if (AWindow)
		AWindow->removeEventFilter(instance());
}

bool WindowSticker::eventFilter(QObject *AWatched, QEvent *AEvent)
{
	if (AEvent->type() == QEvent::NonClientAreaMouseButtonPress)
	{
		QWidget *window = qobject_cast<QWidget *>(AWatched);
		if (window && window->isWindow())
			FCurWindow = window;
	}
	else if (AEvent->type() == QEvent::NonClientAreaMouseButtonRelease)
	{
		FCurWindow = NULL;
	}
	else if (AEvent->type() == QEvent::NonClientAreaMouseMove)
	{
		FCurWindow = NULL;
	}
	else if (AEvent->type() == QEvent::WindowStateChange)
	{
		FCurWindow = NULL;
	}
	else if (AWatched==FCurWindow && AEvent->type()==QEvent::Move)
	{
		const int delta = 15;
		QPoint cursorPos = QCursor::pos();
		QRect windowRect = FCurWindow->frameGeometry();
		QRect desckRect = FCurWindow->screen()->availableGeometry();

		int borderTop = cursorPos.y() - windowRect.y();
		int borderLeft = cursorPos.x() - windowRect.x();
		int borderRight = cursorPos.x() + desckRect.right() - windowRect.right();
		int borderBottom = cursorPos.y() + desckRect.bottom() - windowRect.bottom();

		FStickPos = windowRect.topLeft();
		if (qAbs(borderTop - cursorPos.y()) < delta)
		{
			FStickPos.setY(0);
		}
		else if (qAbs(borderBottom - cursorPos.y()) < delta)
		{
			FStickPos.setY(desckRect.bottom() - windowRect.height());
		}
		if (qAbs(borderLeft - cursorPos.x()) < delta)
		{
			FStickPos.setX(0);
		}
		else if(qAbs(borderRight - cursorPos.x()) < delta)
		{
			FStickPos.setX(desckRect.right() - windowRect.width());
		}

		if (FStickPos != windowRect.topLeft())
		{
			QEvent *stickEvent = new QEvent((QEvent::Type)FStickEvent);
			QApplication::postEvent(AWatched,stickEvent,Qt::HighEventPriority);
		}
	}
	else if (FCurWindow==AWatched && AEvent->type()==FStickEvent)
	{
		FCurWindow->move(FStickPos);
		return true;
	}
	return QObject::eventFilter(AWatched,AEvent);
}

// WidgetManager
struct WidgetManager::WidgetManagerData {
	WidgetManagerData() {
		isAlertEnabled = true;
	}
	bool isAlertEnabled;
};

WidgetManager::WidgetManager()
{
	d = new WidgetManagerData;
}

WidgetManager::~WidgetManager()
{
	delete d;
}

WidgetManager *WidgetManager::instance()
{
	static WidgetManager *inst = new WidgetManager;
	return inst;
}

void WidgetManager::raiseWidget(QWidget *AWidget)
{
	AWidget->raise();
}

bool WidgetManager::isActiveWindow(const QWidget *AWindow)
{
	const QWidget *topWindow = AWindow->window();
	return topWindow->isActiveWindow() && topWindow->isVisible() && !topWindow->isMinimized();
}

void WidgetManager::showActivateRaiseWindow(QWidget *AWindow)
{
	if (AWindow->isVisible())
	{
		if (AWindow->isMinimized())
		{
			if (AWindow->isMaximized())
				AWindow->showMaximized();
			else
				AWindow->showNormal();
		}
	}
	else
	{
		AWindow->show();
	}
	AWindow->activateWindow();
	WidgetManager::raiseWidget(AWindow);
}

void WidgetManager::setWindowSticky( QWidget *AWindow, bool ASticky )
{
#ifdef Q_OS_WIN
	if (ASticky)
		WindowSticker::insertWindow(AWindow);
	else
		WindowSticker::removeWindow(AWindow);
#else
	Q_UNUSED(AWindow);
	Q_UNUSED(ASticky);
#endif
}

void WidgetManager::alertWidget(QWidget *AWidget)
{
	if (AWidget!=NULL && isWidgetAlertEnabled())
		QApplication::alert(AWidget);
}

bool WidgetManager::isWidgetAlertEnabled()
{
	return instance()->d->isAlertEnabled;
}

void WidgetManager::setWidgetAlertEnabled(bool AEnabled)
{
	instance()->d->isAlertEnabled = AEnabled;
}

Qt::Alignment WidgetManager::windowAlignment(const QWidget *AWindow)
{
	Qt::Alignment align;
	QRect windowRect = AWindow->frameGeometry();
	QRect screenRect = AWindow->screen()->availableGeometry();
	if (!screenRect.isEmpty() && !windowRect.isEmpty())
	{
		static const int delta = 4;
		if (qAbs(screenRect.left() - windowRect.left()) < delta)
			align |= Qt::AlignLeft;
		else if (qAbs(screenRect.right() - windowRect.right()) < delta)
			align |= Qt::AlignRight;
		if (qAbs(screenRect.top() - windowRect.top()) < delta)
			align |= Qt::AlignTop;
		else if (qAbs(screenRect.bottom() - windowRect.bottom()) < delta)
			align |= Qt::AlignBottom;
	}
	return align;
}

bool WidgetManager::alignWindow(QWidget *AWindow, Qt::Alignment AAlign)
{
	if (AWindow!=NULL && AAlign>0)
	{
		QRect frameRect = AWindow->frameGeometry();
		QRect windowRect = AWindow->geometry();
		if (!frameRect.isEmpty() && !windowRect.isEmpty() && frameRect.contains(windowRect))
		{
			QRect availRect = AWindow->screen()->availableGeometry();
			QRect rect = alignRect(frameRect,availRect,AAlign);
			rect.adjust(windowRect.left()-frameRect.left(),windowRect.top()-frameRect.top(),windowRect.right()-frameRect.right(),windowRect.bottom()-frameRect.bottom());
			AWindow->setGeometry(rect);
			return true;
		}
	}
	return false;
}

QRect WidgetManager::alignRect(const QRect &ARect, const QRect &ABoundary, Qt::Alignment AAlign)
{
	QRect rect = ARect;
	if (AAlign>0 && !ARect.isEmpty() && !ABoundary.isEmpty())
	{
		if ((AAlign & Qt::AlignLeft) == Qt::AlignLeft)
			rect.moveLeft(ABoundary.left());
		else if ((AAlign & Qt::AlignRight) == Qt::AlignRight)
			rect.moveRight(ABoundary.right());
		else if ((AAlign & Qt::AlignHCenter) == Qt::AlignHCenter)
			rect.moveLeft((ABoundary.width()-ARect.width())/2);

		if ((AAlign & Qt::AlignTop) == Qt::AlignTop)
			rect.moveTop(ABoundary.top());
		else if ((AAlign & Qt::AlignBottom) == Qt::AlignBottom)
			rect.moveBottom(ABoundary.bottom());
		else if ((AAlign & Qt::AlignVCenter) == Qt::AlignVCenter)
			rect.moveTop((ABoundary.height() - ARect.height())/2);
	}
	return rect;
}

QRect WidgetManager::alignGeometry(const QSize &ASize, const QWidget *AWidget, Qt::Alignment AAlign)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
	QRect availRect = AWidget!=NULL ? AWidget->screen()->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	QRect availRect = AWidget!=NULL ? QApplication::desktop()->availableGeometry(AWidget) : QApplication::primaryScreen()->availableGeometry();
#else
	QRect availRect = AWidget!=NULL ? QApplication::desktop()->availableGeometry(AWidget) : QApplication::desktop()->availableGeometry();
#endif
	return QStyle::alignedRect(Qt::LeftToRight,AAlign,ASize.boundedTo(availRect.size()),availRect);
}
