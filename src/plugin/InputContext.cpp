#include "InputContext.h"
#include <QQuickItem>

#include <QInputMethodQueryEvent>
#include <QGuiApplication>
#include <QDebug>
#include <QTime>

InputContext::InputContext(const QString &mainFile):
	m_visible(false),
	m_mainFile(mainFile)
{
	if (m_mainFile.isEmpty()) {
		m_mainFile = "qrc:/quickkeyboard/keyboard.qml";
	}
}

InputContext::~InputContext()
{
}

bool InputContext::isValid() const
{
    return true;
}

void InputContext::update(Qt::InputMethodQueries queries)
{
    if (m_focusObject) {
        QInputMethodQueryEvent event(Qt::ImEnabled);
        QGuiApplication::sendEvent(m_focusObject, &event);
        bool inputMethodEnabled = event.value(Qt::ImEnabled).toBool();

        qWarning() << "Updating input context. InputMethod enabled: " << inputMethodEnabled;
        qWarning() << "Focus object: " << m_focusObject;
        QQuickItem *myItem = qobject_cast<QQuickItem *>(m_focusObject);
        if (myItem) {
            if (!myItem->isVisible()) {
                hideInputPanel();
            }
        }
    }

}

void InputContext::showInputPanel()
{
	m_visible = true;
	QPlatformInputContext::showInputPanel();
	emitInputPanelVisibleChanged();
}

void InputContext::hideInputPanel()
{
	m_visible = false;
	QPlatformInputContext::hideInputPanel();
	emitInputPanelVisibleChanged();
}

bool InputContext::isInputPanelVisible() const
{
	return m_focusObject && m_visible;
}

void InputContext::setFocusObject(QObject *object)
{
    qWarning() << "Calling setFocusObject";
	m_focusObject = object;
    bool focusObjectHasInputMethodEnabled=false;
    bool cond=false;

    if (m_focusObject) {
    QInputMethodQueryEvent event(Qt::ImEnabled);
    QGuiApplication::sendEvent(m_focusObject, &event);
    bool focusObjectHasInputMethodEnabled = event.value(Qt::ImEnabled).toBool();

    cond = (m_focusObject!=0 && focusObjectHasInputMethodEnabled);

    qWarning() << "cond: " << cond;
    qWarning() << "Change focus object, InputMethod enabled: " << focusObjectHasInputMethodEnabled;

    qWarning() << "Focus object: " << m_focusObject;

    }

    qWarning() << "m_visible: " << m_visible;


	if (m_visible) {
        qWarning() << "Scond: " << (m_focusObject!=0 && focusObjectHasInputMethodEnabled);

        qWarning() << "SChange focus object, InputMethod enabled: " << focusObjectHasInputMethodEnabled;

        qWarning() << "SFocus object: " << m_focusObject;
        if (cond /*m_focusObject && focusObjectHasInputMethodEnabled*/) {
			showInputPanel();
            qWarning() << "show from setfocusobject";

		}
		else {
            qWarning() << "hide from setfocusobject";

			hideInputPanel();
		}
	}
	emit focusObjectChanged(m_focusObject);
}

QString InputContext::mainFile() const
{
	return m_mainFile;
}

