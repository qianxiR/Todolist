#include "MainWindow.h"

#include "QuadrantListWidget.h"
#include "TaskCard.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

namespace {
struct MatrixCell {
    TaskQuadrant quadrant;
    int row;
    int column;
};

const std::array<QString, 4> kQuadrantDescriptions = {
    QStringLiteral("立即处理"),
    QStringLiteral("安排时间"),
    QStringLiteral("可委派处理"),
    QStringLiteral("降低投入")
};

const std::array<MatrixCell, 4> kMatrixCells = {{
    {TaskQuadrant::Schedule, 0, 0},
    {TaskQuadrant::DoNow, 0, 1},
    {TaskQuadrant::Eliminate, 1, 0},
    {TaskQuadrant::Delegate, 1, 1}
}};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tasks_(storage_.load())
{
    // 入参：父控件。方法：恢复本地任务、构造主界面并初始化托盘。出参：可在前台或后台运行的主窗口。
    createInterface();
    createTrayIcon();
    refreshTaskLists();
}

void MainWindow::createInterface()
{
    // 入参：无。方法：组织品牌区、启动项开关及坐标轴式四象限矩阵。出参：无。
    setWindowTitle(QStringLiteral("Todolist"));
    setMinimumSize(820, 560);
    resize(920, 630);

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(22, 18, 22, 20);
    root->setSpacing(15);

    auto *clockMark = new QLabel(central);
    clockMark->setPixmap(QIcon(QStringLiteral(":/icons/clock.svg")).pixmap(38, 38));
    clockMark->setFixedSize(38, 38);

    auto *heading = new QLabel(QStringLiteral("TODOLIST"), central);
    heading->setObjectName(QStringLiteral("heading"));
    QFont headingFont(QStringLiteral("Microsoft YaHei UI"));
    headingFont.setPointSize(21);
    headingFont.setWeight(QFont::DemiBold);
    headingFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
    heading->setFont(headingFont);

    auto *caption = new QLabel(QStringLiteral("EISENHOWER MATRIX · 让注意力有处可去"), central);
    caption->setObjectName(QStringLiteral("caption"));

    auto *titleStack = new QVBoxLayout;
    titleStack->setContentsMargins(0, 0, 0, 0);
    titleStack->setSpacing(1);
    titleStack->addWidget(heading);
    titleStack->addWidget(caption);

    auto *brand = new QHBoxLayout;
    brand->setContentsMargins(0, 0, 0, 0);
    brand->setSpacing(9);
    brand->addWidget(clockMark);
    brand->addLayout(titleStack);

    autoStartBox_ = new QCheckBox(QStringLiteral("开机启动"), central);
    autoStartBox_->setObjectName(QStringLiteral("autoStartBox"));
    autoStartBox_->setChecked(isAutoStartEnabled());
    connect(autoStartBox_, &QCheckBox::toggled, this, [this](bool enabled) { setAutoStartEnabled(enabled); });
    if (autoStartBox_->isChecked()) {
        setAutoStartEnabled(true);
    }

    auto *addButton = new QPushButton(QStringLiteral("+ 新建任务"), central);
    addButton->setObjectName(QStringLiteral("addButton"));
    addButton->setCursor(Qt::PointingHandCursor);
    connect(addButton, &QPushButton::clicked, this, [this] { openTaskEditor(TaskQuadrant::DoNow); });

    auto *topBar = new QHBoxLayout;
    topBar->addLayout(brand);
    topBar->addStretch();
    topBar->addWidget(autoStartBox_);
    topBar->addSpacing(10);
    topBar->addWidget(addButton);
    root->addLayout(topBar);

    auto *matrixFrame = new QFrame(central);
    matrixFrame->setObjectName(QStringLiteral("matrixFrame"));
    auto *matrix = new QGridLayout(matrixFrame);
    matrix->setContentsMargins(1, 1, 1, 1);
    matrix->setHorizontalSpacing(1);
    matrix->setVerticalSpacing(1);

    auto *importantAxis = new QLabel(QStringLiteral("重\n要\n↑"), matrixFrame);
    importantAxis->setObjectName(QStringLiteral("importanceAxis"));
    importantAxis->setAlignment(Qt::AlignCenter);
    matrix->addWidget(importantAxis, 0, 1);

    auto *notImportantAxis = new QLabel(QStringLiteral("↓\n不\n重\n要"), matrixFrame);
    notImportantAxis->setObjectName(QStringLiteral("importanceAxis"));
    notImportantAxis->setAlignment(Qt::AlignCenter);
    matrix->addWidget(notImportantAxis, 2, 1);

    for (const MatrixCell &cell : kMatrixCells) {
        const int index = quadrantIndex(cell.quadrant);
        auto *panel = new QFrame(matrixFrame);
        panel->setObjectName(QStringLiteral("quadrantPanel"));
        panel->setProperty("quadrant", index);

        auto *title = new QLabel(quadrantTitle(cell.quadrant), panel);
        title->setObjectName(QStringLiteral("quadrantTitle"));
        auto *description = new QLabel(kQuadrantDescriptions.at(index), panel);
        description->setObjectName(QStringLiteral("quadrantDescription"));

        auto *list = new QuadrantListWidget(cell.quadrant, panel);
        list->setObjectName(QStringLiteral("taskList"));
        list->setFrameShape(QFrame::NoFrame);
        list->setSpacing(8);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        connect(list, &QuadrantListWidget::taskDropped, this, [this](const QString &taskId, TaskQuadrant target, int targetRow) {
            moveTask(taskId, target, targetRow);
        });
        taskLists_.at(index) = list;

        auto *panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(13, 11, 13, 12);
        panelLayout->setSpacing(3);
        panelLayout->addWidget(title);
        panelLayout->addWidget(description);
        panelLayout->addSpacing(8);
        panelLayout->addWidget(list, 1);
        matrix->addWidget(panel, cell.row * 2, cell.column * 2);
    }

    auto *notUrgentAxis = new QLabel(QStringLiteral("← 非紧急"), matrixFrame);
    notUrgentAxis->setObjectName(QStringLiteral("urgencyAxis"));
    notUrgentAxis->setAlignment(Qt::AlignCenter);
    auto *urgentAxis = new QLabel(QStringLiteral("紧急 →"), matrixFrame);
    urgentAxis->setObjectName(QStringLiteral("urgencyAxis"));
    urgentAxis->setAlignment(Qt::AlignCenter);
    auto *axisCenter = new QLabel(QStringLiteral("＋"), matrixFrame);
    axisCenter->setObjectName(QStringLiteral("axisCenter"));
    axisCenter->setAlignment(Qt::AlignCenter);
    matrix->addWidget(notUrgentAxis, 1, 0);
    matrix->addWidget(axisCenter, 1, 1);
    matrix->addWidget(urgentAxis, 1, 2);
    matrix->setColumnMinimumWidth(1, 42);
    matrix->setColumnStretch(0, 1);
    matrix->setColumnStretch(2, 1);
    matrix->setRowStretch(0, 1);
    matrix->setRowStretch(2, 1);
    root->addWidget(matrixFrame, 1);

    setCentralWidget(central);
    setStyleSheet(QStringLiteral(R"(
        QMainWindow { background: #FFFFFF; font-family: "Microsoft YaHei UI", "Segoe UI"; font-size: 13px; }
        QLabel#heading { color: #173F52; }
        QLabel#caption { color: #829BA5; font-size: 11px; font-weight: 600; }
        QCheckBox#autoStartBox { color: #496875; font-size: 12px; font-weight: 600; spacing: 7px; }
        QCheckBox#autoStartBox::indicator { width: 17px; height: 17px; border: 1px solid #B7CBD3; border-radius: 4px; background: #FFFFFF; }
        QCheckBox#autoStartBox::indicator:checked { background: #6FC69B; border-color: #6FC69B; }
        QPushButton#addButton { background: #4FAFCE; border: 0; border-radius: 7px; color: #FFFFFF; font-size: 12px; font-weight: 600; padding: 8px 13px; }
        QPushButton#addButton:hover { background: #349BBC; }
        QFrame#matrixFrame { background: #BCD4DD; border: 1px solid #BCD4DD; border-radius: 10px; }
        QFrame#quadrantPanel { border: 0; }
        QFrame#quadrantPanel[quadrant="0"] { background: #E5F6FB; }
        QFrame#quadrantPanel[quadrant="1"] { background: #ECF8F0; }
        QFrame#quadrantPanel[quadrant="2"] { background: #F1F6F8; }
        QFrame#quadrantPanel[quadrant="3"] { background: #F4FAF2; }
        QLabel#importanceAxis, QLabel#urgencyAxis { background: #F8FCFD; color: #668895; font-size: 11px; font-weight: 600; }
        QLabel#axisCenter { background: #F8FCFD; color: #60AFC8; font-size: 16px; font-weight: 600; }
        QLabel#quadrantTitle { color: #214F63; font-size: 14px; font-weight: 600; }
        QLabel#quadrantDescription { color: #829BA5; font-size: 11px; font-weight: 600; }
        QListWidget#taskList { background: transparent; outline: 0; }
        QListWidget#taskList[dropActive="true"] { background: rgba(111, 196, 219, 0.22); border: 1px dashed #5FAFC7; border-radius: 6px; }
        QListWidget#taskList::item { border: 0; }
        QScrollBar:vertical { background: #EDF5F7; width: 9px; margin: 5px 2px; border-radius: 4px; }
        QScrollBar::handle:vertical { background: #9CCBD9; min-height: 34px; border-radius: 4px; }
        QScrollBar::handle:vertical:hover { background: #60B2CA; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QFrame#taskCard { background: #FFFFFF; border: 1px solid #D7E7EA; border-radius: 7px; }
        QFrame#taskCard[completed="true"] { background: #F0FAF4; border-color: #BDE5CD; }
        QFrame#taskCard:hover { border-color: #84C6D9; }
        QLabel#taskTitle { color: #294B58; font-size: 13px; font-weight: 600; }
        QLabel#noteLabel { color: #73969A; font-size: 11px; font-weight: 600; }
        QPushButton#statusButton { border: 0; border-radius: 5px; font-size: 11px; font-weight: 600; min-width: 52px; padding: 5px 7px; }
        QPushButton#statusButton[completed="false"] { background: #E5F3F8; color: #36748A; }
        QPushButton#statusButton[completed="true"] { background: #DDF3E6; color: #327851; }
        QPushButton#statusButton:hover { background: #CFEAF3; }
        QToolButton { border: 0; border-radius: 4px; color: #557884; padding: 4px; }
        QToolButton:hover { background: #E0F2F7; }
        QDialog { background: #FFFFFF; }
        QLineEdit, QTextEdit, QComboBox { background: #FFFFFF; border: 1px solid #C5D8DE; border-radius: 5px; min-height: 27px; padding: 2px 7px; }
        QDialogButtonBox QPushButton { background: #4FAFCE; border: 0; border-radius: 5px; color: #FFFFFF; min-width: 78px; padding: 7px 10px; }
    )"));
}

void MainWindow::createTrayIcon()
{
    // 入参：无。方法：创建托盘图标及显示、退出操作。出参：无。
    trayIcon_ = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/clock.svg")), this);
    trayIcon_->setToolTip(QStringLiteral("Todolist 正在后台运行"));

    auto *trayMenu = new QMenu(this);
    QAction *restoreAction = trayMenu->addAction(QStringLiteral("显示 Todolist"));
    QAction *quitAction = trayMenu->addAction(QStringLiteral("退出"));
    trayIcon_->setContextMenu(trayMenu);

    connect(restoreAction, &QAction::triggered, this, &MainWindow::restoreWindow);
    connect(quitAction, &QAction::triggered, this, [this] {
        isExplicitExit_ = true;
        QApplication::quit();
    });
    connect(trayIcon_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            restoreWindow();
        }
    });
    trayIcon_->show();
}

void MainWindow::refreshTaskLists()
{
    // 入参：无。方法：清空四个列表后按任务象限重建卡片。出参：无。
    for (QListWidget *list : taskLists_) {
        list->clear();
    }

    for (const TodoTask &task : tasks_) {
        QListWidget *list = taskLists_.at(quadrantIndex(task.quadrant));
        auto *item = new QListWidgetItem(list);
        item->setSizeHint(QSize(0, 70));
        auto *card = new TaskCard(task, list);
        list->setItemWidget(item, card);
        connect(card, &TaskCard::taskUpdated, this, [this](const TodoTask &updated) { replaceTask(updated); });
        connect(card, &TaskCard::editRequested, this, [this](const TodoTask &selected) { openTaskEditor(selected.quadrant, &selected); });
        connect(card, &TaskCard::removeRequested, this, [this](const QString &id) { removeTask(id); });
    }
}

void MainWindow::openTaskEditor(TaskQuadrant initialQuadrant, const TodoTask *existingTask)
{
    // 入参：默认象限及可选既有任务。方法：展示任务表单并保存用户确认的改动。出参：无。
    QDialog dialog(this);
    dialog.setWindowTitle(existingTask == nullptr ? QStringLiteral("新建任务") : QStringLiteral("编辑任务"));
    dialog.setMinimumWidth(360);

    auto *titleEdit = new QLineEdit(&dialog);
    titleEdit->setPlaceholderText(QStringLiteral("输入待办事项"));
    titleEdit->setText(existingTask == nullptr ? QString() : existingTask->title);

    auto *noteEdit = new QTextEdit(&dialog);
    noteEdit->setPlaceholderText(QStringLiteral("输入备注（可选）"));
    noteEdit->setPlainText(existingTask == nullptr ? QString() : existingTask->note);
    noteEdit->setFixedHeight(76);

    auto *quadrantBox = new QComboBox(&dialog);
    for (int index = 0; index < 4; ++index) {
        quadrantBox->addItem(quadrantTitle(quadrantFromIndex(index)));
    }
    quadrantBox->setCurrentIndex(existingTask == nullptr ? quadrantIndex(initialQuadrant) : quadrantIndex(existingTask->quadrant));

    auto *completionBox = new QComboBox(&dialog);
    completionBox->addItem(QStringLiteral("待完成"), false);
    completionBox->addItem(QStringLiteral("已完成"), true);
    completionBox->setCurrentIndex(existingTask != nullptr && existingTask->completed ? 1 : 0);

    auto *form = new QFormLayout;
    form->setSpacing(12);
    form->addRow(QStringLiteral("事项"), titleEdit);
    form->addRow(QStringLiteral("备注"), noteEdit);
    form->addRow(QStringLiteral("四象限"), quadrantBox);
    form->addRow(QStringLiteral("完成状态"), completionBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(22, 20, 22, 20);
    layout->setSpacing(18);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted || titleEdit->text().trimmed().isEmpty()) {
        return;
    }

    TodoTask task = existingTask == nullptr
        ? TodoTask::create(titleEdit->text().trimmed(), noteEdit->toPlainText().trimmed(), quadrantFromIndex(quadrantBox->currentIndex()))
        : *existingTask;
    task.title = titleEdit->text().trimmed();
    task.note = noteEdit->toPlainText().trimmed();
    task.quadrant = quadrantFromIndex(quadrantBox->currentIndex());
    task.completed = completionBox->currentData().toBool();

    if (existingTask == nullptr) {
        tasks_.append(task);
    } else {
        replaceTask(task);
        return;
    }
    saveTasks();
    refreshTaskLists();
}

void MainWindow::replaceTask(const TodoTask &task)
{
    // 入参：带相同标识符的更新任务。方法：替换内存集合中的原任务并持久化。出参：无。
    const auto iterator = std::find_if(tasks_.begin(), tasks_.end(), [&task](const TodoTask &candidate) {
        return candidate.id == task.id;
    });
    if (iterator == tasks_.end()) {
        return;
    }
    *iterator = task;
    saveTasks();
    refreshTaskLists();
}

void MainWindow::moveTask(const QString &taskId, TaskQuadrant targetQuadrant, int targetRow)
{
    // 入参：任务标识、目标象限和插入行。方法：移除源任务后按目标行重排并持久化。出参：无。
    const auto iterator = std::find_if(tasks_.begin(), tasks_.end(), [&taskId](const TodoTask &candidate) {
        return candidate.id == taskId;
    });
    if (iterator == tasks_.end()) {
        return;
    }

    const int sourceIndex = static_cast<int>(std::distance(tasks_.begin(), iterator));
    const TaskQuadrant sourceQuadrant = iterator->quadrant;
    int sourceRow = 0;
    for (int index = 0; index < sourceIndex; ++index) {
        sourceRow += tasks_.at(index).quadrant == targetQuadrant ? 1 : 0;
    }
    if (sourceQuadrant == targetQuadrant && sourceRow < targetRow) {
        --targetRow;
    }

    TodoTask movedTask = *iterator;
    movedTask.quadrant = targetQuadrant;
    tasks_.removeAt(sourceIndex);

    int targetCount = 0;
    int lastTargetIndex = -1;
    int insertionIndex = tasks_.size();
    for (int index = 0; index < tasks_.size(); ++index) {
        if (tasks_.at(index).quadrant != targetQuadrant) {
            continue;
        }
        if (targetCount == targetRow) {
            insertionIndex = index;
            break;
        }
        lastTargetIndex = index;
        ++targetCount;
    }
    if (targetRow >= targetCount && lastTargetIndex >= 0) {
        insertionIndex = lastTargetIndex + 1;
    }
    tasks_.insert(insertionIndex, movedTask);
    saveTasks();
    refreshTaskLists();
}

void MainWindow::removeTask(const QString &taskId)
{
    // 入参：待删除任务的唯一标识符。方法：从内存集合移除匹配任务并持久化。出参：无。
    const auto newEnd = std::remove_if(tasks_.begin(), tasks_.end(), [&taskId](const TodoTask &task) {
        return task.id == taskId;
    });
    tasks_.erase(newEnd, tasks_.end());
    saveTasks();
    refreshTaskLists();
}

void MainWindow::saveTasks()
{
    // 入参：无。方法：将当前内存任务集合委托给存储层保存。出参：无。
    storage_.save(tasks_);
}

bool MainWindow::isAutoStartEnabled() const
{
    // 入参：无。方法：读取当前用户 Run 注册表项。出参：存在 Todolist 启动值时返回 true。
    const QString registryPath = QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings registry(registryPath, QSettings::NativeFormat);
    return registry.contains(QStringLiteral("Todolist"));
}

void MainWindow::setAutoStartEnabled(bool enabled)
{
    // 入参：自启动开关状态。方法：写入或移除当前用户 Run 注册表值。出参：无。
    const QString registryPath = QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings registry(registryPath, QSettings::NativeFormat);
    if (enabled) {
        const QString executable = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        registry.setValue(QStringLiteral("Todolist"), QStringLiteral("\"%1\"").arg(executable));
    } else {
        registry.remove(QStringLiteral("Todolist"));
    }
    registry.sync();
}

void MainWindow::restoreWindow()
{
    // 入参：无。方法：从托盘恢复窗口并置于前台。出参：无。
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 入参：窗口关闭事件。方法：普通关闭时隐藏到托盘，显式退出时允许进程结束。出参：无。
    if (isExplicitExit_) {
        event->accept();
        return;
    }
    hide();
    event->ignore();
    if (!hasShownTrayHint_) {
        trayIcon_->showMessage(QStringLiteral("Todolist"), QStringLiteral("程序仍在后台运行，可通过托盘图标恢复或退出。"));
        hasShownTrayHint_ = true;
    }
}
