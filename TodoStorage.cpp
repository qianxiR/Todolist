#include "TodoStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

QVector<TodoTask> TodoStorage::load() const
{
    // 入参：无。方法：读取 AppData JSON 数组并恢复任务。出参：文件不存在或格式无效时返回空集合。
    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray()) {
        return {};
    }

    QVector<TodoTask> tasks;
    const QJsonArray items = document.array();
    tasks.reserve(items.size());
    for (const QJsonValue &item : items) {
        tasks.append(TodoTask::fromJson(item.toObject()));
    }
    return tasks;
}

bool TodoStorage::save(const QVector<TodoTask> &tasks) const
{
    // 入参：完整任务集合。方法：使用原子保存文件写入 JSON。出参：提交成功时返回 true。
    const QString path = filePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray items;
    for (const TodoTask &task : tasks) {
        items.append(task.toJson());
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(items).toJson(QJsonDocument::Indented));
    return file.commit();
}

QString TodoStorage::filePath() const
{
    // 入参：无。方法：定位当前用户应用数据目录。出参：待办 JSON 的绝对路径。
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(directory).filePath(QStringLiteral("tasks.json"));
}
