#include "TaskEditorDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

TaskEditorDialog::TaskEditorDialog(TaskQuadrant initialQuadrant, const TodoTask *existingTask, QWidget *parent)
    : QDialog(parent), existingTask_(existingTask), initialQuadrant_(initialQuadrant)
{
    // 入参：默认象限、可选既有任务和父控件。方法：构造可维护计划项的任务编辑表单。出参：可执行的新建或编辑对话框。
    setWindowTitle(existingTask_ == nullptr ? QStringLiteral("新建任务") : QStringLiteral("编辑任务"));
    setMinimumWidth(450);

    auto *dialogTitle = new QLabel(windowTitle(), this);
    dialogTitle->setObjectName(QStringLiteral("dialogTitle"));
    auto *dialogSubtitle = new QLabel(QStringLiteral("为任务维护可独立完成的计划项"), this);
    dialogSubtitle->setObjectName(QStringLiteral("dialogSubtitle"));
    dialogSubtitle->setWordWrap(true);

    auto *header = new QVBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(3);
    header->addWidget(dialogTitle);
    header->addWidget(dialogSubtitle);

    auto *divider = new QFrame(this);
    divider->setObjectName(QStringLiteral("dialogDivider"));
    divider->setFixedHeight(1);

    titleEdit_ = new QLineEdit(this);
    titleEdit_->setPlaceholderText(QStringLiteral("输入待办事项"));
    titleEdit_->setText(existingTask_ == nullptr ? QString() : existingTask_->title);
    titleEdit_->setCursorPosition(0);

    auto *plansHost = new QWidget(this);
    planRowsLayout_ = new QVBoxLayout(plansHost);
    planRowsLayout_->setContentsMargins(0, 0, 0, 0);
    planRowsLayout_->setSpacing(6);

    auto *addPlanButton = new QToolButton(plansHost);
    addPlanButton->setObjectName(QStringLiteral("addPlanButton"));
    addPlanButton->setText(QStringLiteral("+"));
    addPlanButton->setToolTip(QStringLiteral("添加计划项"));
    addPlanButton->setCursor(Qt::PointingHandCursor);
    planRowsLayout_->addWidget(addPlanButton, 0, Qt::AlignLeft);
    connect(addPlanButton, &QToolButton::clicked, this, [this] { addPlanRow(); });

    const QVector<TodoPlan> existingPlans = existingTask_ == nullptr ? QVector<TodoPlan>() : existingTask_->plans;
    for (const TodoPlan &plan : existingPlans) {
        addPlanRow(plan);
    }
    if (planRows_.isEmpty()) {
        addPlanRow();
    }

    auto *planScroll = new QScrollArea(this);
    planScroll->setObjectName(QStringLiteral("planEditorScroll"));
    planScroll->setWidgetResizable(true);
    planScroll->setFrameShape(QFrame::NoFrame);
    planScroll->setWidget(plansHost);
    planScroll->setFixedHeight(132);

    quadrantBox_ = new QComboBox(this);
    for (int index = 0; index < 4; ++index) {
        quadrantBox_->addItem(quadrantTitle(quadrantFromIndex(index)));
    }
    quadrantBox_->setCurrentIndex(quadrantIndex(existingTask_ == nullptr ? initialQuadrant_ : existingTask_->quadrant));

    completionBox_ = new QCheckBox(QStringLiteral("标记任务为已完成"), this);
    completionBox_->setObjectName(QStringLiteral("completionEditor"));
    completionBox_->setChecked(existingTask_ != nullptr && existingTask_->completed);
    connect(completionBox_, &QCheckBox::toggled, this, [this](bool completed) {
        setPlanRowsCompleted(completed);
    });

    auto *form = new QVBoxLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(7);
    auto *titleLabel = new QLabel(QStringLiteral("任务"), this);
    titleLabel->setObjectName(QStringLiteral("fieldLabel"));
    auto *plansLabel = new QLabel(QStringLiteral("计划"), this);
    plansLabel->setObjectName(QStringLiteral("fieldLabel"));
    auto *quadrantLabel = new QLabel(QStringLiteral("所属象限"), this);
    quadrantLabel->setObjectName(QStringLiteral("fieldLabel"));
    form->addWidget(titleLabel);
    form->addWidget(titleEdit_);
    form->addSpacing(4);
    form->addWidget(plansLabel);
    form->addWidget(planScroll);
    form->addSpacing(4);
    form->addWidget(quadrantLabel);
    form->addWidget(quadrantBox_);
    form->addSpacing(4);
    form->addWidget(completionBox_);

    auto *cancelButton = new QPushButton(QStringLiteral("取消"), this);
    cancelButton->setObjectName(QStringLiteral("dialogSecondary"));
    auto *saveButton = new QPushButton(existingTask_ == nullptr ? QStringLiteral("创建任务") : QStringLiteral("保存修改"), this);
    saveButton->setObjectName(QStringLiteral("dialogPrimary"));
    saveButton->setDefault(true);
    saveButton->setEnabled(!titleEdit_->text().trimmed().isEmpty());
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(titleEdit_, &QLineEdit::textChanged, saveButton, [saveButton](const QString &text) {
        saveButton->setEnabled(!text.trimmed().isEmpty());
    });

    auto *buttons = new QHBoxLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(8);
    buttons->addStretch();
    buttons->addWidget(cancelButton);
    buttons->addWidget(saveButton);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(16);
    layout->addLayout(header);
    layout->addWidget(divider);
    layout->addLayout(form);
    layout->addLayout(buttons);
}

TodoTask TaskEditorDialog::task() const
{
    // 入参：无。方法：读取表单当前值并组装任务，父任务完成时同步完成全部计划。出参：可保存的完整任务对象。
    TodoTask editedTask = existingTask_ == nullptr
        ? TodoTask::create(titleEdit_->text().trimmed(), plans(), quadrantFromIndex(quadrantBox_->currentIndex()))
        : *existingTask_;
    editedTask.title = titleEdit_->text().trimmed();
    editedTask.plans = plans();
    editedTask.quadrant = quadrantFromIndex(quadrantBox_->currentIndex());
    editedTask.completed = completionBox_->isChecked();
    if (editedTask.completed) {
        for (TodoPlan &plan : editedTask.plans) {
            plan.completed = true;
        }
    }
    return editedTask;
}

void TaskEditorDialog::addPlanRow(const TodoPlan &plan)
{
    // 入参：可选初始计划项。方法：创建具有完成、编辑和删除操作的单行计划编辑控件。出参：无。
    auto *row = new QWidget(this);
    row->setObjectName(QStringLiteral("planEditorRow"));
    auto *completion = new QCheckBox(row);
    completion->setObjectName(QStringLiteral("planCompletionBox"));
    completion->setChecked(plan.completed);
    completion->setToolTip(QStringLiteral("标记计划完成状态"));
    auto *title = new QLineEdit(row);
    title->setObjectName(QStringLiteral("planTitleEdit"));
    title->setPlaceholderText(QStringLiteral("输入具体计划"));
    title->setText(plan.title);
    title->setCursorPosition(0);
    auto *removeButton = new QToolButton(row);
    removeButton->setObjectName(QStringLiteral("removePlanButton"));
    removeButton->setText(QStringLiteral("×"));
    removeButton->setToolTip(QStringLiteral("删除计划项"));
    removeButton->setCursor(Qt::PointingHandCursor);

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);
    layout->addWidget(completion);
    layout->addWidget(title, 1);
    layout->addWidget(removeButton);
    planRowsLayout_->insertWidget(planRows_.size(), row);
    planRows_.append(row);

    connect(removeButton, &QToolButton::clicked, this, [this, row] {
        planRows_.removeOne(row);
        row->deleteLater();
    });
}

QVector<TodoPlan> TaskEditorDialog::plans() const
{
    // 入参：无。方法：收集非空计划行的文本及完成状态。出参：保持编辑顺序的有效计划集合。
    QVector<TodoPlan> result;
    result.reserve(planRows_.size());
    for (QWidget *row : planRows_) {
        const auto *title = row->findChild<QLineEdit *>(QStringLiteral("planTitleEdit"));
        const auto *completion = row->findChild<QCheckBox *>(QStringLiteral("planCompletionBox"));
        const QString planTitle = title->text().trimmed();
        if (!planTitle.isEmpty()) {
            result.append({planTitle, completion->isChecked()});
        }
    }
    return result;
}

void TaskEditorDialog::setPlanRowsCompleted(bool completed)
{
    // 入参：目标完成状态。方法：同步设置当前编辑表单中全部计划行的复选框。出参：无。
    for (QWidget *row : planRows_) {
        row->findChild<QCheckBox *>(QStringLiteral("planCompletionBox"))->setChecked(completed);
    }
}
