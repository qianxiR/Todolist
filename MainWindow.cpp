#include "MainWindow.h"

#include "ApplicationStyle.h"
#include "QuadrantListWidget.h"
#include "TaskCard.h"
#include "TaskEditorDialog.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFont>
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
#include <QVBoxLayout>

#include <algorithm>

namespace {
const std::array<QString, 4> kQuadrantActions = {
    QStringLiteral("立即处理"),
    QStringLiteral("计划安排"),
    QStringLiteral("委派处理"),
    QStringLiteral("暂不处理")
};

const std::array<TaskQuadrant, 4> kQuadrantOrder = {{
    TaskQuadrant::Schedule,
    TaskQuadrant::DoNow,
    TaskQuadrant::Eliminate,
    TaskQuadrant::Delegate
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
    // 入参：无。方法：组织紧凑工具栏与无坐标轴的四象限任务看板。出参：无。
    setWindowTitle(QStringLiteral("Todolist"));
    setMinimumSize(896, 560);
    resize(1152, 720);

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(20, 12, 20, 16);
    root->setSpacing(10);

    auto *clockMark = new QLabel(central);
    clockMark->setPixmap(QIcon(QStringLiteral(":/icons/clock.svg")).pixmap(32, 32));
    clockMark->setFixedSize(32, 32);

    auto *heading = new QLabel(QStringLiteral("Todolist"), central);
    heading->setObjectName(QStringLiteral("heading"));
    QFont headingFont(QStringLiteral("Microsoft YaHei UI"));
    headingFont.setPointSize(18);
    headingFont.setWeight(QFont::DemiBold);
    heading->setFont(headingFont);

    auto *caption = new QLabel(QStringLiteral("四象限任务管理"), central);
    caption->setObjectName(QStringLiteral("caption"));

    auto *titleStack = new QVBoxLayout;
    titleStack->setContentsMargins(0, 0, 0, 0);
    titleStack->setSpacing(0);
    titleStack->addWidget(heading);
    titleStack->addWidget(caption);

    auto *brand = new QHBoxLayout;
    brand->setContentsMargins(0, 0, 0, 0);
    brand->setSpacing(10);
    brand->addWidget(clockMark);
    brand->addLayout(titleStack);

    auto *settingsButton = new QPushButton(QStringLiteral("设置"), central);
    settingsButton->setObjectName(QStringLiteral("settingsButton"));
    settingsButton->setToolTip(QStringLiteral("应用设置"));
    settingsButton->setCursor(Qt::PointingHandCursor);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);

    auto *addButton = new QPushButton(QStringLiteral("+ 新建任务"), central);
    addButton->setObjectName(QStringLiteral("addButton"));
    addButton->setCursor(Qt::PointingHandCursor);
    connect(addButton, &QPushButton::clicked, this, [this] { openTaskEditor(TaskQuadrant::DoNow); });

    auto *topBar = new QHBoxLayout;
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(8);
    topBar->addLayout(brand);
    topBar->addStretch();
    topBar->addWidget(settingsButton);
    topBar->addWidget(addButton);
    root->addLayout(topBar);

    auto *board = new QWidget(central);
    auto *matrix = new QGridLayout(board);
    matrix->setContentsMargins(0, 0, 0, 0);
    matrix->setHorizontalSpacing(12);
    matrix->setVerticalSpacing(12);

    for (int position = 0; position < static_cast<int>(kQuadrantOrder.size()); ++position) {
        const TaskQuadrant quadrant = kQuadrantOrder.at(position);
        const int index = quadrantIndex(quadrant);
        auto *panel = new QFrame(board);
        panel->setObjectName(QStringLiteral("quadrantPanel"));
        panel->setProperty("quadrant", index);

        auto *accent = new QFrame(panel);
        accent->setObjectName(QStringLiteral("quadrantAccent"));
        accent->setProperty("quadrant", index);
        accent->setFixedSize(4, 30);

        auto *title = new QLabel(kQuadrantActions.at(index), panel);
        title->setObjectName(QStringLiteral("quadrantTitle"));
        auto *description = new QLabel(quadrantTitle(quadrant), panel);
        description->setObjectName(QStringLiteral("quadrantDescription"));

        auto *titleStack = new QVBoxLayout;
        titleStack->setContentsMargins(0, 0, 0, 0);
        titleStack->setSpacing(1);
        titleStack->addWidget(title);
        titleStack->addWidget(description);

        auto *countLabel = new QLabel(QStringLiteral("0"), panel);
        countLabel->setObjectName(QStringLiteral("quadrantCount"));
        countLabel->setAlignment(Qt::AlignCenter);
        countLabel->setFixedSize(28, 22);
        quadrantCountLabels_.at(index) = countLabel;

        auto *panelHeader = new QHBoxLayout;
        panelHeader->setContentsMargins(0, 0, 0, 0);
        panelHeader->setSpacing(9);
        panelHeader->addWidget(accent);
        panelHeader->addLayout(titleStack);
        panelHeader->addStretch();
        panelHeader->addWidget(countLabel);

        auto *list = new QuadrantListWidget(quadrant, panel);
        list->setObjectName(QStringLiteral("taskList"));
        list->setFrameShape(QFrame::NoFrame);
        list->setSpacing(8);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        connect(list, &QuadrantListWidget::taskDropped, this, [this](const QString &taskId, TaskQuadrant target, int targetRow) {
            moveTask(taskId, target, targetRow);
        });
        taskLists_.at(index) = list;

        auto *panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(14, 13, 14, 14);
        panelLayout->setSpacing(12);
        panelLayout->addLayout(panelHeader);
        panelLayout->addWidget(list, 1);
        matrix->addWidget(panel, position / 2, position % 2);
    }

    matrix->setColumnStretch(0, 1);
    matrix->setColumnStretch(1, 1);
    matrix->setRowStretch(0, 1);
    matrix->setRowStretch(1, 1);
    root->addWidget(board, 1);

    setCentralWidget(central);
    setStyleSheet(applicationStyleSheet());
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
    // 入参：无。方法：按任务象限重建紧凑卡片并同步各象限计数。出参：无。
    for (QListWidget *list : taskLists_) {
        list->clear();
    }

    std::array<int, 4> quadrantCounts{};
    for (const TodoTask &task : tasks_) {
        const int index = quadrantIndex(task.quadrant);
        QListWidget *list = taskLists_.at(index);
        auto *item = new QListWidgetItem(list);
        item->setSizeHint(QSize(0, TaskCard::displayHeight(task)));
        auto *card = new TaskCard(task, list);
        list->setItemWidget(item, card);
        ++quadrantCounts.at(index);
        connect(card, &TaskCard::taskUpdated, this, [this](const TodoTask &updated) { replaceTask(updated); });
        connect(card, &TaskCard::editRequested, this, [this](const TodoTask &selected) { openTaskEditor(selected.quadrant, &selected); });
        connect(card, &TaskCard::removeRequested, this, [this](const QString &id) { removeTask(id); });
    }

    for (int index = 0; index < static_cast<int>(quadrantCountLabels_.size()); ++index) {
        quadrantCountLabels_.at(index)->setText(QString::number(quadrantCounts.at(index)));
    }
}

void MainWindow::openTaskEditor(TaskQuadrant initialQuadrant, const TodoTask *existingTask)
{
    // 入参：默认象限及可选既有任务。方法：委托计划编辑对话框收集并保存完整任务。出参：无。
    TaskEditorDialog dialog(initialQuadrant, existingTask, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const TodoTask task = dialog.task();
    if (task.title.isEmpty()) {
        return;
    }

    if (existingTask == nullptr) {
        tasks_.append(task);
    } else {
        replaceTask(task);
        refreshTaskLists();
        return;
    }
    saveTasks();
    refreshTaskLists();
}

void MainWindow::openSettings()
{
    // 入参：无。方法：使用统一主题弹窗编辑应用级设置，并在确认后持久化。出参：无。
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("设置"));
    dialog.setMinimumWidth(420);

    auto *dialogTitle = new QLabel(QStringLiteral("设置"), &dialog);
    dialogTitle->setObjectName(QStringLiteral("dialogTitle"));
    auto *dialogSubtitle = new QLabel(QStringLiteral("管理 Todolist 的运行方式"), &dialog);
    dialogSubtitle->setObjectName(QStringLiteral("dialogSubtitle"));
    dialogSubtitle->setWordWrap(true);

    auto *dialogHeader = new QVBoxLayout;
    dialogHeader->setContentsMargins(0, 0, 0, 0);
    dialogHeader->setSpacing(3);
    dialogHeader->addWidget(dialogTitle);
    dialogHeader->addWidget(dialogSubtitle);

    auto *divider = new QFrame(&dialog);
    divider->setObjectName(QStringLiteral("dialogDivider"));
    divider->setFixedHeight(1);

    auto *settingTitle = new QLabel(QStringLiteral("开机启动"), &dialog);
    settingTitle->setObjectName(QStringLiteral("fieldLabel"));
    auto *settingDescription = new QLabel(QStringLiteral("登录 Windows 后自动运行 Todolist"), &dialog);
    settingDescription->setObjectName(QStringLiteral("settingDescription"));
    settingDescription->setWordWrap(true);

    auto *settingText = new QVBoxLayout;
    settingText->setContentsMargins(0, 0, 0, 0);
    settingText->setSpacing(3);
    settingText->addWidget(settingTitle);
    settingText->addWidget(settingDescription);

    auto *autoStartBox = new QCheckBox(QStringLiteral("启用"), &dialog);
    autoStartBox->setObjectName(QStringLiteral("completionEditor"));
    autoStartBox->setChecked(isAutoStartEnabled());

    auto *settingRow = new QHBoxLayout;
    settingRow->setContentsMargins(0, 2, 0, 2);
    settingRow->setSpacing(16);
    settingRow->addLayout(settingText, 1);
    settingRow->addWidget(autoStartBox, 0, Qt::AlignVCenter);

    auto *cancelButton = new QPushButton(QStringLiteral("取消"), &dialog);
    cancelButton->setObjectName(QStringLiteral("dialogSecondary"));
    auto *saveButton = new QPushButton(QStringLiteral("保存设置"), &dialog);
    saveButton->setObjectName(QStringLiteral("dialogPrimary"));
    saveButton->setDefault(true);

    auto *buttons = new QHBoxLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(8);
    buttons->addStretch();
    buttons->addWidget(cancelButton);
    buttons->addWidget(saveButton);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(18);
    layout->addLayout(dialogHeader);
    layout->addWidget(divider);
    layout->addLayout(settingRow);
    layout->addSpacing(4);
    layout->addLayout(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    setAutoStartEnabled(autoStartBox->isChecked());
}

void MainWindow::replaceTask(const TodoTask &task)
{
    // 入参：带相同标识符的更新任务。方法：替换内存集合并持久化，保留当前列表视图与滚动位置。出参：无。
    const auto iterator = std::find_if(tasks_.begin(), tasks_.end(), [&task](const TodoTask &candidate) {
        return candidate.id == task.id;
    });
    if (iterator == tasks_.end()) {
        return;
    }
    *iterator = task;
    saveTasks();
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
