#include "TaskCard.h"

#include <QApplication>
#include <QDrag>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

TaskCard::TaskCard(const TodoTask &task, QWidget *parent)
    : QFrame(parent), task_(task)
{
    // 入参：任务数据和父控件。方法：创建任务摘要、完成状态及操作按钮。出参：可嵌入象限列表的任务卡片。
    setObjectName(QStringLiteral("taskCard"));
    setFrameShape(QFrame::NoFrame);

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName(QStringLiteral("taskTitle"));
    titleLabel_->setWordWrap(true);
    titleLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    noteLabel_ = new QLabel(this);
    noteLabel_->setObjectName(QStringLiteral("noteLabel"));
    noteLabel_->setWordWrap(true);
    noteLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    statusButton_ = new QPushButton(this);
    statusButton_->setObjectName(QStringLiteral("statusButton"));
    statusButton_->setCursor(Qt::PointingHandCursor);

    auto *editButton = new QToolButton(this);
    editButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    editButton->setToolTip(QStringLiteral("编辑任务"));

    auto *removeButton = new QToolButton(this);
    removeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    removeButton->setToolTip(QStringLiteral("删除任务"));

    auto *actions = new QHBoxLayout;
    actions->setContentsMargins(0, 0, 0, 0);
    actions->setSpacing(2);
    actions->addWidget(statusButton_);
    actions->addWidget(editButton);
    actions->addWidget(removeButton);

    auto *content = new QVBoxLayout;
    content->setContentsMargins(0, 0, 0, 0);
    content->setSpacing(2);
    content->addWidget(titleLabel_);
    content->addWidget(noteLabel_);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 7, 7, 7);
    layout->setSpacing(8);
    layout->addLayout(content, 1);
    layout->addLayout(actions, 0);

    connect(statusButton_, &QPushButton::clicked, this, [this] {
        task_.completed = !task_.completed;
        updateAppearance();
        emit taskUpdated(task_);
    });
    connect(editButton, &QToolButton::clicked, this, [this] { emit editRequested(task_); });
    connect(removeButton, &QToolButton::clicked, this, [this] { emit removeRequested(task_.id); });

    updateAppearance();
}

void TaskCard::mousePressEvent(QMouseEvent *event)
{
    // 入参：卡片鼠标按下事件。方法：记录左键拖动起点且保留基类事件处理。出参：无。
    if (event->button() == Qt::LeftButton) {
        dragStartPosition_ = event->pos();
    }
    QFrame::mousePressEvent(event);
}

void TaskCard::mouseMoveEvent(QMouseEvent *event)
{
    // 入参：卡片鼠标移动事件。方法：超过系统拖动阈值后携带任务 ID 发起移动操作。出参：无。
    if (!(event->buttons() & Qt::LeftButton)
        || (event->pos() - dragStartPosition_).manhattanLength() < QApplication::startDragDistance()) {
        QFrame::mouseMoveEvent(event);
        return;
    }

    auto *drag = new QDrag(this);
    auto *mimeData = new QMimeData;
    mimeData->setData("application/x-todolist-task-id", task_.id.toUtf8());
    drag->setMimeData(mimeData);
    drag->setPixmap(grab());
    drag->setHotSpot(event->pos());
    drag->exec(Qt::MoveAction);
}

void TaskCard::updateAppearance()
{
    // 入参：无。方法：依据完成状态更新标题、备注与卡片视觉状态。出参：无。
    QFont titleFont = titleLabel_->font();
    titleFont.setStrikeOut(task_.completed);
    titleLabel_->setFont(titleFont);
    titleLabel_->setText(task_.title);
    noteLabel_->setText(task_.note.trimmed().isEmpty() ? QStringLiteral("暂无备注") : QStringLiteral("备注：%1").arg(task_.note));
    statusButton_->setText(task_.completed ? QStringLiteral("已完成") : QStringLiteral("待完成"));
    statusButton_->setProperty("completed", task_.completed);
    setProperty("completed", task_.completed);
    style()->unpolish(statusButton_);
    style()->polish(statusButton_);
    style()->unpolish(this);
    style()->polish(this);
}
