#include <QtGlobal>
#include "ButtonItem.h"
#include "GridLayoutItem.h"

#include <QDebug>

GridLayoutItemAttached::GridLayoutItemAttached(QObject *parent):
	QObject(parent),
	m_col(0),
	m_row(0),
	m_colSpan(1),
	m_rowSpan(1)
{
}

GridLayoutItemAttached::~GridLayoutItemAttached()
{
}

GridLayoutItem::GridLayoutItem(QQuickItem *parent):
	LayoutItem(parent),
	m_rows(0),
	m_cols(0),
	m_autoSize(true)
{
	setFlag(QQuickItem::ItemHasContents);
	setKeepTouchGrab(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	m_touchPositions << QPointF();
}

GridLayoutItem::~GridLayoutItem()
{
}

void GridLayoutItem::addButton(ButtonItem *button)
{
	LayoutItem::addButton(button);
	if (m_autoSize) {
		setRowsSimple(qMax(m_rows, layoutProperty(button, "row", 0) + layoutProperty(button, "rowSpan", 1)));
		setColsSimple(qMax(m_cols, layoutProperty(button, "col", 0) + layoutProperty(button, "colSpan", 1)));
	}

	QObject *layoutAttached = qmlAttachedPropertiesObject<GridLayoutItem>(button);
	if (layoutAttached) {
		connect(layoutAttached, SIGNAL(colChanged(int)), SLOT(recalculateRowColSize()));
		connect(layoutAttached, SIGNAL(rowChanged(int)), SLOT(recalculateRowColSize()));
		connect(layoutAttached, SIGNAL(colSpanChanged(int)), SLOT(recalculateRowColSize()));
		connect(layoutAttached, SIGNAL(rowSpanChanged(int)), SLOT(recalculateRowColSize()));
	}
}

void GridLayoutItem::clearButtons()
{
	foreach (const ButtonItem *button, buttons()) {
		QObject *layoutAttached = qmlAttachedPropertiesObject<GridLayoutItem>(button);
		if (layoutAttached) {
			disconnect(layoutAttached, SIGNAL(colChanged(int)), this, SLOT(recalculateRowColSize()));
			disconnect(layoutAttached, SIGNAL(rowChanged(int)), this, SLOT(recalculateRowColSize()));
			disconnect(layoutAttached, SIGNAL(colSpanChanged(int)), this, SLOT(recalculateRowColSize()));
			disconnect(layoutAttached, SIGNAL(rowSpanChanged(int)), this, SLOT(recalculateRowColSize()));
		}
	}
	if (m_autoSize) {
		setRowsSimple(0);
		setColsSimple(0);
	}
	LayoutItem::clearButtons();
}

void GridLayoutItem::setCols(int cols)
{
	m_autoSize = false;
	setColsSimple(cols);
}

void GridLayoutItem::setRows(int rows)
{
	m_autoSize = false;
	setRowsSimple(rows);
}

GridLayoutItemAttached *GridLayoutItem::qmlAttachedProperties(QObject *object)
{
	return new GridLayoutItemAttached(object);
}

void GridLayoutItem::setColsSimple(int cols)
{
	if (m_cols == cols) {
		return;
	}

	m_cols = cols;
	recalculatePositions();
}

void GridLayoutItem::synchronizeActivePoints()
{
	foreach (ButtonItem *button, buttons()) {
		bool buttonActive = button->isActive();
		bool pointActive = checkActive(button);
		if (buttonActive && !pointActive) {
			button->setActive(false);
		}
		if (!buttonActive && pointActive) {
			button->setActive(true);
		}
	}
}

void GridLayoutItem::setRowsSimple(int rows)
{
	if (m_rows == rows) {
		return;
	}

	m_rows = rows;
	recalculatePositions();
}

void GridLayoutItem::triggerOnPosition(int x, int y)
{
	QPointF point(x, y);
	foreach (ButtonItem *button, buttons()) {
		QPointF mapped = button->mapFromScene(point);
		if (mapped.x() >= 0.0f && mapped.y() >= 0.0f && mapped.x() < button->width() && mapped.y() < button->height()) {
			emit button->triggered();
		}
	}
}

void GridLayoutItem::setMousePosition(const QPointF &position)
{
	m_touchPositions[0] = position;
	synchronizeActivePoints();
}

void GridLayoutItem::setTouchPositions(const QList<QPointF> &positions)
{
	m_touchPositions = QList<QPointF>() << m_touchPositions[0];
	m_touchPositions += positions;
	synchronizeActivePoints();
}

bool GridLayoutItem::checkActive(const ButtonItem *button) const
{
	foreach (const QPointF &point, m_touchPositions) {
		if (!point.isNull()) {
			QPointF mapped = button->mapFromScene(point);
			if (mapped.x() >= 0.0f && mapped.y() >= 0.0f && mapped.x() < button->width() && mapped.y() < button->height()) {
				return true;
			}
		}
	}
	return false;
}

void GridLayoutItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
	LayoutItem::geometryChanged(newGeometry, oldGeometry);
	recalculatePositions();
}

void GridLayoutItem::touchEvent(QTouchEvent *event)
{
	QVector<QPointF> points;
	points.reserve(event->touchPoints().length());
	foreach (const QTouchEvent::TouchPoint &point, event->touchPoints()) {
		points << point.scenePos();
	}
	setTouchPositions(points.toList());

	QVector<QPointF> pointsAfterRelease;
	pointsAfterRelease.reserve(event->touchPoints().length());
	foreach (const QTouchEvent::TouchPoint &point, event->touchPoints()) {
		if (point.state() == Qt::TouchPointReleased) {
			pointsAfterRelease << QPointF();
			triggerOnPosition(point.pos().x(), point.pos().y());
		}
		else {
			pointsAfterRelease << point.pos().toPoint();
		}
	}

	if (points != pointsAfterRelease) {
		setTouchPositions(pointsAfterRelease.toList());
	}
}

void GridLayoutItem::mouseMoveEvent(QMouseEvent *event)
{
	event->accept();
	setMousePosition(mapToScene(QPointF(event->x(), event->y())));
}

void GridLayoutItem::mousePressEvent(QMouseEvent *event)
{
	event->accept();
	setMousePosition(mapToScene(QPointF(event->x(), event->y())));
}

void GridLayoutItem::mouseReleaseEvent(QMouseEvent *event)
{
	event->accept();
	QPointF point(event->x(), event->y());
	point = mapToScene(point);
	triggerOnPosition(point.x(), point.y());
	setMousePosition(QPointF());
}

int GridLayoutItem::layoutProperty(const ButtonItem *button, const char *property, int fallback)
{
	QObject *layoutAttached = qmlAttachedPropertiesObject<GridLayoutItem>(button);
	if (!layoutAttached) {
		return fallback;
	}
	return layoutAttached->property(property).toInt();
}

void GridLayoutItem::recalculateRowColSize()
{
	if (!m_autoSize) {
		return;
	}
	int rows = 0;
	int cols = 0;
	foreach (const ButtonItem *button, buttons()) {
		rows = qMax(layoutProperty(button, "row", 0) + layoutProperty(button, "rowSpan", 1), rows);
		cols = qMax(layoutProperty(button, "col", 0) + layoutProperty(button, "colSpan", 1), cols);
	}
	setRowsSimple(rows);
	setColsSimple(cols);
}

void GridLayoutItem::recalculatePositions()
{
	if (m_rows == 0 || m_cols == 0) {
		return;
	}

	int w = width();
	int h = height();

	foreach (ButtonItem *button, buttons()) {
		QObject *layoutAttached = qmlAttachedPropertiesObject<GridLayoutItem>(button);
		if (!layoutAttached) {
			continue;
		}

		int left = layoutAttached->property("col").toInt();
		int top = layoutAttached->property("row").toInt();
		int right = layoutAttached->property("colSpan").toInt() + left;
		int bottom = layoutAttached->property("rowSpan").toInt() + top;

		int x = left * w / m_cols;
		int y = top * h / m_rows;

		button->setProperty("x", x);
		button->setProperty("y", y);
		button->setProperty("width", right * w / m_cols - x);
		button->setProperty("height", bottom * h / m_rows - y);
	}
}
