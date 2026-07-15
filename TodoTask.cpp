#include "TodoTask.h"

#include <QUuid>

QJsonObject TodoTask::toJson() const
{
    // 入参：当前任务对象。方法：将可持久化字段映射为 JSON。出参：可直接写入本地文件的 JSON 对象。
    return {
        {"id", id},
        {"title", title},
        {"note", note},
        {"quadrant", quadrantIndex(quadrant)},
        {"completed", completed}
    };
}

TodoTask TodoTask::fromJson(const QJsonObject &json)
{
    // 入参：任务 JSON。方法：恢复字段并为缺失标识符生成唯一值。出参：可在界面中显示的任务对象。
    TodoTask task;
    task.id = json.value("id").toString();
    task.id = task.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : task.id;
    task.title = json.value("title").toString();
    task.note = json.value("note").toString();
    task.quadrant = quadrantFromIndex(json.value("quadrant").toInt());
    task.completed = json.value("completed").toBool();
    return task;
}

TodoTask TodoTask::create(const QString &title, const QString &note, TaskQuadrant quadrant)
{
    // 入参：标题、备注和象限。方法：建立带唯一标识的初始未完成任务。出参：可保存的新任务。
    TodoTask task;
    task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = title;
    task.note = note;
    task.quadrant = quadrant;
    return task;
}

QString quadrantTitle(TaskQuadrant quadrant)
{
    // 入参：任务象限。方法：根据四象限枚举选择界面文案。出参：对应的中文标题。
    switch (quadrant) {
    case TaskQuadrant::DoNow:
        return QStringLiteral("重要且紧急");
    case TaskQuadrant::Schedule:
        return QStringLiteral("重要不紧急");
    case TaskQuadrant::Delegate:
        return QStringLiteral("不重要但紧急");
    case TaskQuadrant::Eliminate:
        return QStringLiteral("不重要不紧急");
    }
    return QString();
}

int quadrantIndex(TaskQuadrant quadrant)
{
    // 入参：任务象限。方法：转换为稳定的存储和控件索引。出参：范围为 0 至 3 的整数。
    return static_cast<int>(quadrant);
}

TaskQuadrant quadrantFromIndex(int index)
{
    // 入参：持久化的象限索引。方法：将非法值收敛为首要象限。出参：有效的任务象限。
    switch (index) {
    case 1:
        return TaskQuadrant::Schedule;
    case 2:
        return TaskQuadrant::Delegate;
    case 3:
        return TaskQuadrant::Eliminate;
    default:
        return TaskQuadrant::DoNow;
    }
}
