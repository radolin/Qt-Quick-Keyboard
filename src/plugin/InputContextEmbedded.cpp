#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQuickItem>
#include <QRectF>
#include <QUrl>
#include "../Dispatcher.h"
#include "../KeyboardItem.h"
#include "../register.h"

#include "InputContextEmbedded.h"

InputContextEmbedded::InputContextEmbedded(const QString &mainFile):
	InputContext(mainFile),
	m_component(0),
	m_keyboard(0)
{
	registerQmlTypes();
    connect(this, SIGNAL(focusObjectChanged(QObject*)), this, SLOT(onFocusObjectChanged(QObject*)));
}

InputContextEmbedded::~InputContextEmbedded()
{
}

QRectF InputContextEmbedded::keyboardRect() const
{
	if (!m_keyboard) {
		return QRectF();
	}
    return m_keyboard->property("geometry").toRectF();
}

void InputContextEmbedded::showInputPanel()
{
    qWarning() << "Showing input panel";

    if (!m_keyboard) {
        m_focusWindow = qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow());
        qWarning() << "class" << m_focusWindow->metaObject()->className() ;
        qWarning() << "parent" <<m_focusWindow->parent();

            if (m_focusWindow) {

                m_keyboard = m_focusWindow->findChild<QQuickItem *>("radokey");

                if (!m_keyboard) {
                    qWarning() << "Can't find keyboard!!!";
                    return;
                }
                qWarning() << "Found keyboard: " <<m_keyboard->metaObject()->className();

                qWarning() << "Found keyboard: " <<m_keyboard->objectName();

                connect(m_keyboard, SIGNAL(geometryChanged()), this, SLOT(onKeyboardRectChanged()));

			}
    }
    qWarning() << "Already set keyboard ";

    InputContext::showInputPanel();
    updateVisibility();
}

void InputContextEmbedded::hideInputPanel()
{
	InputContext::hideInputPanel();
	updateVisibility();
}

void InputContextEmbedded::embedKeyboard()
{/*
	if (m_component->isError()) {
		qWarning() << m_component->errors();
		return;
	}

	m_keyboard = qobject_cast<QQuickItem *>(m_component->beginCreate(m_focusWindow->rootContext()));
	if (!m_keyboard) {
		qWarning() << "Root component is not QQuickItem";
		m_component->completeCreate();
		return;
	}
	QQuickItem *rootObject = m_focusWindow->rootObject();

	m_keyboard->setProperty("parent", QVariant::fromValue(qobject_cast<QQuickItem *>(m_focusWindow->rootObject())));
	m_keyboard->setParent(rootObject);
	m_keyboard->setParentItem(rootObject);

	QQuickItem *content = rootObject->findChild<QQuickItem *>("content");
	if (content) {
		m_keyboard->setProperty("content", QVariant::fromValue<QQuickItem *>(content));
	}
	updateVisibility();

	m_component->completeCreate();
    connect(m_keyboard, SIGNAL(geometryChanged()), this, SLOT(onKeyboardRectChanged()));*/
}

void InputContextEmbedded::onFocusObjectChanged(QObject *focusObject)
{
	if (m_keyboard) {
		QuickKeyboard::KeyboardItem *kbd = m_keyboard->findChild<QuickKeyboard::KeyboardItem *>("QuickKeyboard");
        if (kbd) {
//             qWarning() << "Focus changed to " << focusObject->metaObject()->className();
        kbd->dispatcher()->setFocusObject(focusObject);
        }
        else  qWarning() << "Couldn't get dispatcher";

	}
}

void InputContextEmbedded::updateVisibility()
{
	if (m_keyboard) {
         qWarning() << "Updating visibility offf" << m_keyboard->objectName() << " to " <<isInputPanelVisible();
		m_keyboard->setProperty("isVisible", isInputPanelVisible());
    } else {
        qWarning() << "Couldn't update visibility, no m_keyboard!!!" ;
    }
}

void InputContextEmbedded::onKeyboardRectChanged()
{
	emitKeyboardRectChanged();
}
