#pragma once

#include "TodoTask.h"

#include <QVector>

class TodoStorage final {
public:
    QVector<TodoTask> load() const;
    bool save(const QVector<TodoTask> &tasks) const;

private:
    QString filePath() const;
};
