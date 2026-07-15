#pragma once

#include "TodoStorage.h"

#include <QMainWindow>

#include <array>

class QCheckBox;
class QCloseEvent;
class QSystemTrayIcon;
class QuadrantListWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createInterface();
    void createTrayIcon();
    void refreshTaskLists();
    void openTaskEditor(TaskQuadrant initialQuadrant, const TodoTask *existingTask = nullptr);
    void replaceTask(const TodoTask &task);
    void moveTask(const QString &taskId, TaskQuadrant targetQuadrant, int targetRow);
    void removeTask(const QString &taskId);
    void saveTasks();
    bool isAutoStartEnabled() const;
    void setAutoStartEnabled(bool enabled);
    void restoreWindow();
    void closeEvent(QCloseEvent *event) override;

    TodoStorage storage_;
    QVector<TodoTask> tasks_;
    std::array<QuadrantListWidget *, 4> taskLists_{};
    QSystemTrayIcon *trayIcon_ = nullptr;
    QCheckBox *autoStartBox_ = nullptr;
    bool hasShownTrayHint_ = false;
    bool isExplicitExit_ = false;
};
