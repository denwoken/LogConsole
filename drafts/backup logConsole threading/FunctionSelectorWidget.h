#ifndef FUNCTIONSELECTORWIDGET //_H
#define FUNCTIONSELECTORWIDGET _H
#include "qdialog.h"


class QCheckBox;
class QLineEdit;
class QMenu;
class QVBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QStandardItemModel;
class QStandardItem;

namespace Logging
{

class LogConsoleWidget;

/*!
 * \brief The FunctionSelectorWidget class класс диалогового окна для выбора
 *  функций для отображения, а также других правил сортировки
 */
class FunctionSelectorWidget : public QDialog
{
public:
    explicit FunctionSelectorWidget(LogConsoleWidget* parent = nullptr);
    ~FunctionSelectorWidget();
    /*!
     * \brief addFunction добавляет функцию или namespace в древо функций
     *  по формату namespace::function
     */
    void addFunction(const QString& path);
    /*!
     * \brief setCheckStateFunction устанавливает состояние checkbox
     *  у соответствующей функции
     */
    void setCheckStateFunction(const QString& path, bool st);
    /*!
     * \brief sortItems сортирует функции по названию в древе
     */
    void sortItems();


    /*!
     * \brief convertToStandardItemModel конвертирует QTreeWidget в QStandardItemModel
     * с которым будет работать сортировщик для ускорения
     */
    QStandardItemModel* convertToStandardItemModel();
    /*!
     * \brief convertToVector конвертирует древо в вектор
     *  для удобного сохранения в файл
     */
    QVector<QPair<QString, bool>> convertToVector();

    void updateEnablesLogMsgs();

private:
    LogConsoleWidget* m_parent;
    QLineEdit* m_searchField;
    QCheckBox* m_checkAll;
    QTreeWidget* m_treeWidget;
    QLineEdit* m_addField;
    QMenu *m_logLevelMenu;
    QPushButton* m_logLevelButton;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;

    /*!
     * \brief findOrCreateItem находит элемент по имени среди наследников или создает его
     */
    QTreeWidgetItem* findOrCreateItem(QTreeWidgetItem* parentItem, const QString& name);
    /*!
     * \brief addCheckBoxToItem создает QCheckBox во втором столбце QTreeWidgetItem
     */
    QCheckBox* addCheckBoxToItem(QTreeWidgetItem* item);
    void updateChildCheckBoxes(QTreeWidgetItem* item); // обновляет QCheckBox-ы наследников
    void updateParentCheckBox(QTreeWidgetItem* item); // обновляет QCheckBox родителя

    void removeItem(QTreeWidgetItem* item);
    /*!
     * \brief filterTree отфильтровать древо по ветвям
     *  содержащим данный текст(имя0)     */
    void filterTree(const QString& text);
    /*!
     * \brief searchTreeElement рекурсивный поиск элементов в древе
     * скрывает ветки если нет совпадений */
    int searchTreeElement(QTreeWidgetItem* item, const QString& text);


    void convertTreeWidgetToStandardItemModel(QStandardItem* parentItem, QTreeWidgetItem* treeWidgetItem);
    void convertTreeWidgetToVector(QVector<QPair<QString, bool>>& parentItem, QString func, QTreeWidgetItem* treeWidgetItem);
};

};//Logging

#endif // FUNCTIONSELECTORWIDGET _H
