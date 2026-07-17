#include "TodoTask.h"

#include <QJsonArray>
#include <QStringList>
#include <QUuid>

QJsonObject TodoPlan::toJson() const
{
    // 入参：当前计划项。方法：将计划内容及完成状态映射为 JSON。出参：可保存的计划 JSON 对象。
    return {
        {"title", title},
        {"completed", completed}
    };
}

TodoPlan TodoPlan::fromJson(const QJsonObject &json)
{
    // 入参：计划 JSON。方法：恢复并清理计划文本及完成状态。出参：供任务过滤和显示的计划项。
    TodoPlan plan;
    plan.title = json.value("title").toString().trimmed();
    plan.completed = json.value("completed").toBool();
    return plan;
}

QJsonObject TodoTask::toJson() const
{
    // 入参：当前任务对象。方法：将任务属性与计划项集合映射为 JSON。出参：可直接写入本地文件的 JSON 对象。
    QJsonArray planItems;
    for (const TodoPlan &plan : plans) {
        planItems.append(plan.toJson());
    }
    return {
        {"id", id},
        {"title", title},
        {"plans", planItems},
        {"quadrant", quadrantIndex(quadrant)},
        {"completed", completed}
    };
}

TodoTask TodoTask::fromJson(const QJsonObject &json)
{
    // 入参：任务 JSON。方法：恢复字段、计划项并兼容迁移旧版备注。出参：可在界面中显示的任务对象。
    TodoTask task;
    task.id = json.value("id").toString();
    task.id = task.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : task.id;
    task.title = json.value("title").toString();
    const QJsonArray planItems = json.value("plans").toArray();
    task.plans.reserve(planItems.size());
    for (const QJsonValue &item : planItems) {
        const TodoPlan plan = TodoPlan::fromJson(item.toObject());
        if (!plan.title.isEmpty()) {
            task.plans.append(plan);
        }
    }
    if (task.plans.isEmpty() && json.contains("note")) {
        const QStringList legacyPlans = json.value("note").toString().split('\n', Qt::SkipEmptyParts);
        for (const QString &legacyPlan : legacyPlans) {
            const QString title = legacyPlan.trimmed();
            if (!title.isEmpty()) {
                task.plans.append({title, false});
            }
        }
    }
    task.quadrant = quadrantFromIndex(json.value("quadrant").toInt());
    task.completed = json.value("completed").toBool();
    if (task.completed) {
        for (TodoPlan &plan : task.plans) {
            plan.completed = true;
        }
    }
    return task;
}

TodoTask TodoTask::create(const QString &title, const QVector<TodoPlan> &plans, TaskQuadrant quadrant)
{
    // 入参：标题、计划项与象限。方法：建立带唯一标识的初始未完成任务。出参：可保存的新任务。
    TodoTask task;
    task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = title;
    task.plans = plans;
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
