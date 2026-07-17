#include "TaskCard.h"

#include <QApplication>
#include <QAction>
#include <QCheckBox>
#include <QDrag>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QStyle>
#include <QToolButton>
#include <QTimer>
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

    planSummaryLabel_ = new QLabel(this);
    planSummaryLabel_->setObjectName(QStringLiteral("planSummary"));
    planSummaryLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto *plans = new QWidget(this);
    plans->setObjectName(QStringLiteral("planList"));
    planLayout_ = new QVBoxLayout(plans);
    planLayout_->setContentsMargins(0, 0, 0, 0);
    planLayout_->setSpacing(2);

    completionBox_ = new QCheckBox(this);
    completionBox_->setObjectName(QStringLiteral("completionBox"));
    completionBox_->setCursor(Qt::PointingHandCursor);
    completionBox_->setToolTip(QStringLiteral("标记为完成"));

    auto *moreButton = new QToolButton(this);
    moreButton->setObjectName(QStringLiteral("moreButton"));
    moreButton->setText(QStringLiteral("•••"));
    moreButton->setToolTip(QStringLiteral("更多操作"));
    moreButton->setPopupMode(QToolButton::InstantPopup);
    moreButton->setCursor(Qt::PointingHandCursor);

    auto *menu = new QMenu(moreButton);
    QAction *editAction = menu->addAction(QStringLiteral("编辑任务"));
    QAction *removeAction = menu->addAction(QStringLiteral("删除任务"));
    moreButton->setMenu(menu);

    auto *content = new QVBoxLayout;
    content->setContentsMargins(0, 0, 0, 0);
    content->setSpacing(4);
    content->addWidget(titleLabel_);
    content->addWidget(planSummaryLabel_);
    content->addWidget(plans);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 8, 8);
    layout->setSpacing(10);
    layout->addWidget(completionBox_, 0, Qt::AlignTop);
    layout->addLayout(content, 1);
    layout->addWidget(moreButton, 0, Qt::AlignTop);

    connect(completionBox_, &QCheckBox::toggled, this, [this](bool completed) {
        task_.completed = completed;
        setPlansCompleted(completed);
        updateAppearance();
        QTimer::singleShot(0, this, [this] { emit taskUpdated(task_); });
    });
    connect(editAction, &QAction::triggered, this, [this] {
        QTimer::singleShot(0, this, [this] { emit editRequested(task_); });
    });
    connect(removeAction, &QAction::triggered, this, [this] {
        QTimer::singleShot(0, this, [this] { emit removeRequested(task_.id); });
    });

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
    // 入参：无。方法：依据任务及计划完成状态重建卡片摘要与计划清单。出参：无。
    QFont titleFont = titleLabel_->font();
    titleFont.setStrikeOut(task_.completed);
    titleLabel_->setFont(titleFont);
    titleLabel_->setText(task_.title);

    while (QLayoutItem *item = planLayout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    int completedPlans = 0;
    for (const TodoPlan &plan : task_.plans) {
        completedPlans += plan.completed ? 1 : 0;
    }
    for (int index = 0; index < task_.plans.size(); ++index) {
        const TodoPlan &plan = task_.plans.at(index);

        auto *planRow = new QWidget(this);
        planRow->setObjectName(QStringLiteral("planRow"));
        auto *planBox = new QCheckBox(planRow);
        planBox->setObjectName(QStringLiteral("planCompletionBox"));
        planBox->setChecked(plan.completed);
        planBox->setCursor(Qt::PointingHandCursor);
        planBox->setToolTip(plan.completed ? QStringLiteral("标记计划为待完成") : QStringLiteral("标记计划为完成"));
        auto *planTitle = new QLabel(plan.title, planRow);
        planTitle->setObjectName(QStringLiteral("planTitle"));
        planTitle->setWordWrap(true);
        planTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
        QFont planFont = planTitle->font();
        planFont.setStrikeOut(plan.completed);
        planTitle->setFont(planFont);
        auto *planRowLayout = new QHBoxLayout(planRow);
        planRowLayout->setContentsMargins(0, 0, 0, 0);
        planRowLayout->setSpacing(6);
        planRowLayout->addWidget(planBox, 0, Qt::AlignTop);
        planRowLayout->addWidget(planTitle, 1);
        connect(planBox, &QCheckBox::toggled, this, [this, index](bool completed) {
            task_.plans[index].completed = completed;
            updateAppearance();
            QTimer::singleShot(0, this, [this] { emit taskUpdated(task_); });
        });
        planLayout_->addWidget(planRow);
    }
    planSummaryLabel_->setVisible(!task_.plans.isEmpty());
    planSummaryLabel_->setText(QStringLiteral("计划 %1/%2 已完成").arg(completedPlans).arg(task_.plans.size()));
    completionBox_->blockSignals(true);
    completionBox_->setChecked(task_.completed);
    completionBox_->blockSignals(false);
    completionBox_->setToolTip(task_.completed ? QStringLiteral("标记为待完成") : QStringLiteral("标记为完成"));
    setProperty("completed", task_.completed);
    style()->unpolish(this);
    style()->polish(this);
}

void TaskCard::setPlansCompleted(bool completed)
{
    // 入参：目标完成状态。方法：同步设置父任务下全部计划项的完成状态。出参：无。
    for (TodoPlan &plan : task_.plans) {
        plan.completed = completed;
    }
}
