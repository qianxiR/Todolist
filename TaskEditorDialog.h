#pragma once

#include "TodoTask.h"

#include <QDialog>
#include <QVector>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QVBoxLayout;
class QWidget;

class TaskEditorDialog final : public QDialog {
    Q_OBJECT

public:
    explicit TaskEditorDialog(TaskQuadrant initialQuadrant, const TodoTask *existingTask, QWidget *parent = nullptr);
    TodoTask task() const;

private:
    void addPlanRow(const TodoPlan &plan = {});
    QVector<TodoPlan> plans() const;
    void setPlanRowsCompleted(bool completed);

    const TodoTask *existingTask_ = nullptr;
    TaskQuadrant initialQuadrant_ = TaskQuadrant::DoNow;
    QLineEdit *titleEdit_ = nullptr;
    QComboBox *quadrantBox_ = nullptr;
    QCheckBox *completionBox_ = nullptr;
    QVBoxLayout *planRowsLayout_ = nullptr;
    QVector<QWidget *> planRows_;
};
