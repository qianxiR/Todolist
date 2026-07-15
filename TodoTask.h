#pragma once

#include <QJsonObject>
#include <QString>

enum class TaskQuadrant {
    DoNow,
    Schedule,
    Delegate,
    Eliminate
};

struct TodoTask {
    QString id;
    QString title;
    QString note;
    TaskQuadrant quadrant = TaskQuadrant::DoNow;
    bool completed = false;

    QJsonObject toJson() const;
    static TodoTask fromJson(const QJsonObject &json);
    static TodoTask create(const QString &title, const QString &note, TaskQuadrant quadrant);
};

QString quadrantTitle(TaskQuadrant quadrant);
int quadrantIndex(TaskQuadrant quadrant);
TaskQuadrant quadrantFromIndex(int index);
