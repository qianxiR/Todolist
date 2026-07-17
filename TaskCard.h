#pragma once

#include "TodoTask.h"

#include <QFrame>
#include <QPoint>

class QCheckBox;
class QLabel;
class QMouseEvent;
class QVBoxLayout;

class TaskCard final : public QFrame {
    Q_OBJECT

public:
    explicit TaskCard(const TodoTask &task, QWidget *parent = nullptr);
    static int displayHeight(const TodoTask &task, int cardWidth);

signals:
    void taskUpdated(const TodoTask &task);
    void editRequested(const TodoTask &task);
    void removeRequested(const QString &taskId);

private:
    void updateAppearance();
    void setPlansCompleted(bool completed);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    TodoTask task_;
    QCheckBox *completionBox_ = nullptr;
    QLabel *titleLabel_ = nullptr;
    QLabel *planSummaryLabel_ = nullptr;
    QVBoxLayout *planLayout_ = nullptr;
    QPoint dragStartPosition_;
};
