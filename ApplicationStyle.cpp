#include "ApplicationStyle.h"

QString applicationStyleSheet()
{
    // 入参：无。方法：集中定义主窗口与任务编辑界面的视觉层级和交互状态。出参：可直接应用到主窗口的 Qt 样式表。
    return QStringLiteral(R"(
        QMainWindow { background: #FFFFFF; font-family: "Microsoft YaHei UI", "Segoe UI"; font-size: 13px; }
        QLabel#heading { color: #172B35; }
        QLabel#caption { color: #8A99A1; font-size: 11px; }
        QPushButton#settingsButton, QPushButton#addButton { background: #2F93B4; border: 0; border-radius: 6px; color: #FFFFFF; font-size: 12px; font-weight: 600; padding: 9px 14px; }
        QPushButton#settingsButton:hover, QPushButton#addButton:hover { background: #287F9B; }
        QFrame#quadrantPanel { background: #F7F9FA; border: 1px solid #E2E8EA; border-radius: 8px; }
        QFrame#quadrantAccent { border: 0; border-radius: 2px; }
        QFrame#quadrantAccent[quadrant="0"] { background: #DF665B; }
        QFrame#quadrantAccent[quadrant="1"] { background: #389E78; }
        QFrame#quadrantAccent[quadrant="2"] { background: #D29335; }
        QFrame#quadrantAccent[quadrant="3"] { background: #8B98A0; }
        QLabel#quadrantTitle { color: #233941; font-size: 14px; font-weight: 600; }
        QLabel#quadrantDescription { color: #8A999F; font-size: 10px; }
        QLabel#quadrantCount { background: #E9EEF0; border-radius: 11px; color: #65777E; font-size: 11px; font-weight: 600; }
        QListWidget#taskList { background: transparent; outline: 0; }
        QListWidget#taskList[dropActive="true"] { background: rgba(47, 147, 180, 0.09); border: 1px dashed #5EA9C0; border-radius: 6px; }
        QListWidget#taskList::item { border: 0; }
        QScrollBar:vertical { background: transparent; width: 8px; margin: 4px 1px; }
        QScrollBar::handle:vertical { background: #C8D2D6; min-height: 34px; border-radius: 4px; }
        QScrollBar::handle:vertical:hover { background: #AAB9BF; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QFrame#taskCard { background: #FFFFFF; border: 1px solid #E1E7E9; border-radius: 6px; }
        QFrame#taskCard[completed="true"] { background: #F8FAFA; }
        QFrame#taskCard:hover { border-color: #AABBC1; }
        QLabel#taskTitle { color: #263C44; font-size: 13px; font-weight: 600; }
        QLabel#planSummary { color: #829198; font-size: 10px; }
        QWidget#planRow { background: transparent; }
        QLabel#planTitle { color: #61737B; font-size: 11px; }
        QCheckBox#completionBox { spacing: 0; }
        QCheckBox#completionBox::indicator { width: 17px; height: 17px; border: 1px solid #AEBCC1; border-radius: 5px; background: #FFFFFF; }
        QCheckBox#completionBox::indicator:hover { border-color: #2F93B4; }
        QCheckBox#completionBox::indicator:checked, QCheckBox#planCompletionBox::indicator:checked { background: #389E78; border-color: #389E78; }
        QCheckBox#planCompletionBox { color: #61737B; font-size: 11px; spacing: 6px; }
        QCheckBox#planCompletionBox::indicator { width: 13px; height: 13px; border: 1px solid #B8C5C9; border-radius: 4px; background: #FFFFFF; }
        QToolButton#moreButton { border: 0; border-radius: 4px; color: #73848B; font-size: 12px; font-weight: 600; padding: 3px 5px; }
        QToolButton#moreButton:hover { background: #EDF2F3; color: #314850; }
        QToolButton#moreButton::menu-indicator { image: none; width: 0; }
        QMenu { background: #FFFFFF; border: 1px solid #DCE4E7; border-radius: 6px; padding: 5px; }
        QMenu::item { border-radius: 4px; color: #344A52; padding: 7px 26px 7px 10px; }
        QMenu::item:selected { background: #EDF4F6; }
        QDialog { background: #FFFFFF; }
        QLabel#dialogTitle { color: #1F353E; font-size: 15px; font-weight: 600; }
        QLabel#dialogSubtitle { color: #87969C; font-size: 10px; }
        QLabel#fieldLabel { color: #40555D; font-size: 11px; font-weight: 600; }
        QLabel#settingDescription { color: #87969C; font-size: 11px; }
        QFrame#dialogDivider { background: #E7ECEE; border: 0; max-height: 1px; }
        QLineEdit, QComboBox { background: #FFFFFF; border: 1px solid #CBD6DA; border-radius: 5px; color: #263C44; padding: 4px 8px; selection-background-color: #74B5CA; }
        QLineEdit:focus, QComboBox:focus { border-color: #2F93B4; }
        QLineEdit { min-height: 26px; }
        QComboBox { min-height: 26px; }
        QCheckBox#completionEditor { color: #40555D; spacing: 8px; }
        QCheckBox#completionEditor::indicator { width: 17px; height: 17px; border: 1px solid #AEBCC1; border-radius: 5px; background: #FFFFFF; }
        QCheckBox#completionEditor::indicator:checked { background: #389E78; border-color: #389E78; }
        QScrollArea#planEditorScroll { background: #F8FAFA; border: 1px solid #E1E7E9; border-radius: 5px; }
        QScrollArea#planEditorScroll > QWidget > QWidget, QWidget#planEditorRow { background: transparent; }
        QLineEdit#planTitleEdit { font-size: 11px; min-height: 24px; }
        QToolButton#addPlanButton, QToolButton#removePlanButton { border: 0; border-radius: 4px; color: #5A737D; font-size: 16px; min-width: 24px; min-height: 24px; }
        QToolButton#addPlanButton:hover { background: #E2F0F4; color: #287F9B; }
        QToolButton#removePlanButton:hover { background: #FBE8E6; color: #C5544B; }
        QPushButton#dialogPrimary { background: #2F93B4; border: 0; border-radius: 6px; color: #FFFFFF; font-weight: 600; min-width: 86px; padding: 9px 14px; }
        QPushButton#dialogPrimary:hover { background: #287F9B; }
        QPushButton#dialogPrimary:disabled { background: #B7CDD5; }
        QPushButton#dialogSecondary { background: #FFFFFF; border: 1px solid #D4DEE1; border-radius: 6px; color: #4A5E66; min-width: 78px; padding: 8px 13px; }
        QPushButton#dialogSecondary:hover { background: #F3F6F7; }
    )");
}
