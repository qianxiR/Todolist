#include "QuadrantListWidget.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStyle>

namespace {
constexpr char kTaskMimeType[] = "application/x-todolist-task-id";
}

QuadrantListWidget::QuadrantListWidget(TaskQuadrant quadrant, QWidget *parent)
    : QListWidget(parent), quadrant_(quadrant)
{
    // 入参：目标象限和父控件。方法：配置列表作为仅接收任务拖放数据的目标区域。出参：可高亮的象限列表控件。
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setDragDropMode(QAbstractItemView::DropOnly);
}

void QuadrantListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    // 入参：拖入事件。方法：校验任务 MIME 类型并激活目标高亮。出参：无。
    if (!event->mimeData()->hasFormat(kTaskMimeType)) {
        event->ignore();
        return;
    }
    setDropActive(true);
    event->acceptProposedAction();
}

void QuadrantListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // 入参：拖动经过事件。方法：持续接受有效任务数据。出参：无。
    if (!event->mimeData()->hasFormat(kTaskMimeType)) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
}

void QuadrantListWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    // 入参：拖离事件。方法：移除目标高亮。出参：无。
    setDropActive(false);
    event->accept();
}

void QuadrantListWidget::dropEvent(QDropEvent *event)
{
    // 入参：拖放事件。方法：提取任务标识和插入行并通知主窗口重排。出参：无。
    setDropActive(false);
    if (!event->mimeData()->hasFormat(kTaskMimeType)) {
        event->ignore();
        return;
    }
    const QString taskId = QString::fromUtf8(event->mimeData()->data(kTaskMimeType));
    if (!taskId.isEmpty()) {
        QListWidgetItem *targetItem = itemAt(event->pos());
        int targetRow = targetItem == nullptr ? count() : row(targetItem);
        if (targetItem != nullptr && event->pos().y() > visualItemRect(targetItem).center().y()) {
            ++targetRow;
        }
        emit taskDropped(taskId, quadrant_, targetRow);
    }
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void QuadrantListWidget::setDropActive(bool active)
{
    // 入参：高亮开关。方法：更新动态样式属性并立即重抛光控件。出参：无。
    setProperty("dropActive", active);
    style()->unpolish(this);
    style()->polish(this);
}
