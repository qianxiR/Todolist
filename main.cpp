#include "MainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    // 入参：进程启动参数。方法：初始化应用、配置托盘常驻并显示主窗口。出参：事件循环退出码。
    QApplication application(argc, argv);
    application.setOrganizationName(QStringLiteral("TodoLIST"));
    application.setApplicationName(QStringLiteral("Todolist"));
    application.setQuitOnLastWindowClosed(false);
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/clock.svg")));

    MainWindow window;
    window.show();
    return application.exec();
}
