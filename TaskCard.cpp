#include "TaskCard.h"

#include <QApplication>
#include <QCheckBox>
#include <QDrag>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPushButton>
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

    auto *editButton = new QToolButton(this);
    editButton->setObjectName(QStringLiteral("editTaskButton"));
    editButton->setIcon(QIcon(QStringLiteral(":/icons/edit.svg")));
    editButton->setIconSize(QSize(16, 16));
    editButton->setToolTip(QStringLiteral("编辑任务"));
    editButton->setCursor(Qt::PointingHandCursor);

    auto *removeButton = new QToolButton(this);
    removeButton->setObjectName(QStringLiteral("removeTaskButton"));
    removeButton->setIcon(QIcon(QStringLiteral(":/icons/delete.svg")));
    removeButton->setIconSize(QSize(16, 16));
    removeButton->setToolTip(QStringLiteral("删除任务"));
    removeButton->setCursor(Qt::PointingHandCursor);

    auto *header = new QHBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(8);
    header->addWidget(completionBox_, 0, Qt::AlignVCenter);
    header->addWidget(titleLabel_, 1, Qt::AlignVCenter);
    header->addWidget(editButton, 0, Qt::AlignTop);
    header->addWidget(removeButton, 0, Qt::AlignTop);

    auto *content = new QVBoxLayout;
    content->setContentsMargins(0, 0, 0, 0);
    content->setSpacing(4);
    content->addLayout(header);
    content->addWidget(planSummaryLabel_);
    content->addWidget(plans);
    content->addStretch(1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->addLayout(content);

    connect(completionBox_, &QCheckBox::toggled, this, [this](bool completed) {
        task_.completed = completed;
        setPlansCompleted(completed);
        QTimer::singleShot(0, this, [this] {
            updateAppearance();
            emit taskUpdated(task_);
        });
    });
    connect(editButton, &QToolButton::clicked, this, [this] {
        QTimer::singleShot(0, this, [this] { emit editRequested(task_); });
    });
    connect(removeButton, &QToolButton::clicked, this, [this] {
        QMessageBox confirmation(this);
        confirmation.setIcon(QMessageBox::Warning);
        confirmation.setWindowTitle(QStringLiteral("确认删除"));
        confirmation.setText(QStringLiteral("确认删除任务“%1”？").arg(task_.title));
        confirmation.setInformativeText(QStringLiteral("删除后将无法恢复。"));
        auto *cancelButton = confirmation.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
        auto *confirmButton = confirmation.addButton(QStringLiteral("删除"), QMessageBox::DestructiveRole);
        confirmation.setDefaultButton(cancelButton);
        confirmation.exec();
        if (confirmation.clickedButton() != confirmButton) {
            return;
        }
        QTimer::singleShot(0, this, [this] { emit removeRequested(task_.id); });
    });

    updateAppearance();
}

int TaskCard::displayHeight(const TodoTask &task, int cardWidth)
{
    // 入参：待显示任务和当前卡片宽度。方法：按标题与计划文本的实际可用宽度累计换行高度。出参：恰好容纳卡片内容的列表项高度。
    constexpr int kHorizontalMargins = 24;
    constexpr int kHeaderControlsWidth = 17 + 24 + 24 + 3 * 8;
    constexpr int kPlanControlWidth = 13 + 6;
    const int titleWidth = qMax(120, cardWidth - kHorizontalMargins - kHeaderControlsWidth);
    const int planWidth = qMax(120, cardWidth - kHorizontalMargins - kPlanControlWidth);
    const auto textHeight = [](const QFontMetrics &metrics, const QString &text, int width) {
        return metrics.boundingRect(QRect(0, 0, width, 0), Qt::TextWordWrap, text).height();
    };
    const QFontMetrics titleMetrics(QFont(QStringLiteral("Microsoft YaHei UI"), 10, QFont::DemiBold));
    const QFontMetrics planMetrics(QFont(QStringLiteral("Microsoft YaHei UI"), 9));
    const QFontMetrics summaryMetrics(QFont(QStringLiteral("Microsoft YaHei UI"), 8));
    int height = 16 + qMax(24, textHeight(titleMetrics, task.title, titleWidth));
    if (!task.plans.isEmpty()) {
        height += 8 + qMax(12, textHeight(summaryMetrics, QStringLiteral("计划 0/0 已完成"), planWidth));
        for (int index = 0; index < task.plans.size(); ++index) {
            if (index > 0) {
                height += 2;
            }
            height += qMax(17, textHeight(planMetrics, task.plans.at(index).title, planWidth));
        }
    }
    return height;
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
            QTimer::singleShot(0, this, [this] {
                updateAppearance();
                emit taskUpdated(task_);
            });
        });
        planLayout_->addWidget(planRow);
    }
    planLayout_->addStretch(1);
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
