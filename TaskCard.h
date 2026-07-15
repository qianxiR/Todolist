#pragma once

#include "TodoTask.h"

#include <QFrame>
#include <QPoint>

class QLabel;
class QMouseEvent;
class QPushButton;

class TaskCard final : public QFrame {
    Q_OBJECT

public:
    explicit TaskCard(const TodoTask &task, QWidget *parent = nullptr);

signals:
    void taskUpdated(const TodoTask &task);
    void editRequested(const TodoTask &task);
    void removeRequested(const QString &taskId);

private:
    void updateAppearance();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    TodoTask task_;
    QPushButton *statusButton_ = nullptr;
    QLabel *titleLabel_ = nullptr;
    QLabel *noteLabel_ = nullptr;
    QPoint dragStartPosition_;
};
