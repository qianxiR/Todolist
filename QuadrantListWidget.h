#pragma once

#include "TodoTask.h"

#include <QListWidget>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;

class QuadrantListWidget final : public QListWidget {
    Q_OBJECT

public:
    explicit QuadrantListWidget(TaskQuadrant quadrant, QWidget *parent = nullptr);

signals:
    void taskDropped(const QString &taskId, TaskQuadrant quadrant, int targetRow);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setDropActive(bool active);

    TaskQuadrant quadrant_;
};
