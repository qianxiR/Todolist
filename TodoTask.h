#pragma once

#include <QJsonObject>
#include <QString>
#include <QVector>

enum class TaskQuadrant {
    DoNow,
    Schedule,
    Delegate,
    Eliminate
};

struct TodoPlan {
    QString title;
    bool completed = false;

    QJsonObject toJson() const;
    static TodoPlan fromJson(const QJsonObject &json);
};

struct TodoTask {
    QString id;
    QString title;
    QVector<TodoPlan> plans;
    TaskQuadrant quadrant = TaskQuadrant::DoNow;
    bool completed = false;

    QJsonObject toJson() const;
    static TodoTask fromJson(const QJsonObject &json);
    static TodoTask create(const QString &title, const QVector<TodoPlan> &plans, TaskQuadrant quadrant);
};

QString quadrantTitle(TaskQuadrant quadrant);
int quadrantIndex(TaskQuadrant quadrant);
TaskQuadrant quadrantFromIndex(int index);
