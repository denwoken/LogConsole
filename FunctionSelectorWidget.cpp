#include "FunctionSelectorWidget.h"
#include "LogConsoleWidget.h"
#include "qboxlayout.h"
#include "qcheckbox.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "qpushbutton.h"
#include "qstandarditemmodel.h"
#include <QTreeWidget>

using namespace Logging;

FunctionSelectorWidget::FunctionSelectorWidget(LogConsoleWidget* parent):
    QDialog(parent), m_parent(parent) {
    setMinimumSize(250, 300);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);  // Отключаем рамки и включаем режим Popup
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(1);
    layout->setContentsMargins(1,1,1,1);

    QHBoxLayout* Hlayout0 = new QHBoxLayout();
    layout->addLayout(Hlayout0,1);
    // Создаем строку поиска
    m_searchField = new QLineEdit(this);
    m_searchField->setPlaceholderText("Enter class/function name to search...");
    Hlayout0->addWidget(m_searchField);


    //кнопка для открытия меню
    m_logLevelButton = new QPushButton(this);
    m_logLevelButton->setFixedSize(24, 24);
    m_logLevelButton->setIcon(QIcon(":/resources/3vertical-lines.png"));
    Hlayout0->addWidget(m_logLevelButton);


    QHBoxLayout* Hlayout1 = new QHBoxLayout();
    layout->addLayout(Hlayout1, 1);


    // Создание меню
    m_logLevelMenu = new QMenu(this);
    QAction *action1 = new QAction("display Info", m_logLevelMenu);
    action1->setCheckable(true);
    if(m_parent)
        action1->setChecked(m_parent->m_settings.enableLogMsgs.infoMsg);
    m_logLevelMenu->addAction(action1);
    connect(action1, &QAction::toggled, [&](bool state){
        if(m_parent)
            m_parent->m_settings.enableLogMsgs.infoMsg = state;
    });

    QAction *action2 = new QAction("display debug", m_logLevelMenu);
    action2->setCheckable(true);
    if(m_parent)
        action2->setChecked(m_parent->m_settings.enableLogMsgs.debugMsg);
    m_logLevelMenu->addAction(action2);
    connect(action2, &QAction::toggled, [&](bool state){
        if(m_parent)
            m_parent->m_settings.enableLogMsgs.debugMsg = state;
    });

    QAction *action3 = new QAction("display warning", m_logLevelMenu);
    action3->setCheckable(true);
    if(m_parent)
        action3->setChecked(m_parent->m_settings.enableLogMsgs.warningMsg);
    m_logLevelMenu->addAction(action3);
    connect(action3, &QAction::toggled, [&](bool state){
        if(m_parent)
            m_parent->m_settings.enableLogMsgs.warningMsg = state;
    });

    QAction *action4 = new QAction("display critical", m_logLevelMenu);
    action4->setCheckable(true);
    if(m_parent)
        action4->setChecked(m_parent->m_settings.enableLogMsgs.criticalMsg);
    m_logLevelMenu->addAction(action4);
    connect(action4, &QAction::toggled, [&](bool state){
        if(m_parent)
            m_parent->m_settings.enableLogMsgs.criticalMsg = state;
    });

    QAction *action5 = new QAction("display fatal", m_logLevelMenu);
    action5->setCheckable(true);
    if(m_parent)
        action5->setChecked(m_parent->m_settings.enableLogMsgs.fatalMsg);
    m_logLevelMenu->addAction(action5);
    connect(action5, &QAction::toggled, [&](bool state){
        if(m_parent)
            m_parent->m_settings.enableLogMsgs.fatalMsg = state;
    });



    // Подключаем кнопку к открытию подменю включения уровня логирования
    connect(m_logLevelButton, &QPushButton::clicked, [&]() {
        QPoint pos = QPoint(1 + m_logLevelButton->width() - m_logLevelMenu->width(), m_logLevelButton->height());
        pos = m_logLevelButton->mapToGlobal(pos);
        m_logLevelMenu->move(pos);
        m_logLevelMenu->show();
    });

    // Создаем древо фунукций
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(2);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_treeWidget->setColumnWidth(1, 30);
    m_treeWidget->setHeaderHidden(false);
    //m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setHeaderLabels(QStringList() << "Namespace/Function" << "Active");
    layout->addWidget(m_treeWidget);
    m_treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);


    // инициализация QCheckBox-а включения/выключения всех функций
    m_checkAll = addCheckBoxToItem(m_treeWidget->invisibleRootItem());
    m_checkAll->setText("Отметить все");
    Hlayout1->addWidget(m_checkAll);
    m_checkAll->setChecked(true);
    m_checkAll->setTristate(true);
    connect(m_checkAll, &QCheckBox::clicked, [=]() {
        if(m_checkAll->checkState() == Qt::PartiallyChecked) m_checkAll->setCheckState(Qt::Checked);
    });
    connect(m_checkAll, &QCheckBox::stateChanged, [=]() {
        updateChildCheckBoxes(m_treeWidget->invisibleRootItem());
    });



    QHBoxLayout* Hlayout = new QHBoxLayout();
    layout->addLayout(Hlayout,1);
    Hlayout->setContentsMargins(1,1,1,1);

    m_addField = new QLineEdit(this);
    m_addField->setPlaceholderText("Enter the name of the class/function to add");
    Hlayout->addWidget(m_addField);

    //кнопка добавления айтема (функции)
    m_addButton = new QPushButton(this);
    m_addButton->setFixedSize(24, 24);
    m_addButton->setIcon(QIcon(":/resources/plus_icon.png"));
    connect(m_addButton, &QPushButton::clicked, [&](){
        this->addFunction(m_addField->text());
        sortItems();
    });
    Hlayout->addWidget(m_addButton);

    //кнопка удаления выбранного айтема (функции)
    m_removeButton = new QPushButton(this);
    m_removeButton->setFixedSize(24, 24);
    m_removeButton->setIcon(QIcon(":/resources/minus_icon.png"));
    connect(m_removeButton, &QPushButton::clicked, [&](){
        QList<QTreeWidgetItem*> items = m_treeWidget->selectedItems();
        for(QTreeWidgetItem* item : items){
            this->removeItem(item);
        }
    });
    Hlayout->addWidget(m_removeButton);

    // Подключаем сигнал сортировки по названию при изменении строки поиска
    connect(m_searchField, &QLineEdit::textChanged, this, &FunctionSelectorWidget::filterTree);
}

FunctionSelectorWidget::~FunctionSelectorWidget()
{
    removeItem(m_treeWidget->invisibleRootItem());
}

void FunctionSelectorWidget::addFunction(const QString &path)
{
    QStringList parts = path.split("::");
    if(parts.size() < 0) return;

    QTreeWidgetItem *item = m_treeWidget->invisibleRootItem();
    for(int i = 0; i < parts.size(); i++)
    {
        item = findOrCreateItem(item, parts[i]);
    }
}

void FunctionSelectorWidget::setCheckStateFunction(const QString &path, bool st)
{
    QStringList parts = path.split("::");
    if(parts.size() < 0) return;

    QTreeWidgetItem *item = m_treeWidget->invisibleRootItem();
    for(int i = 0; i < parts.size(); i++)
    {
        QTreeWidgetItem *child = nullptr;
        int childNumLast = item->childCount()-1;
        for(int j = 0; j <= childNumLast; j++)
        {
            child = item->child(j);
            if(!child)continue;
            if(child->text(0) == parts[i]){
                item = child;
                if(j == childNumLast){
                    QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(child, 1));
                    if(checkBox)
                        checkBox->setChecked(st);
                }
                continue;
            }
        }
    }
}

QTreeWidgetItem *FunctionSelectorWidget::findOrCreateItem(QTreeWidgetItem *parentItem, const QString &name)
{
    QTreeWidgetItem *child = nullptr;
    for(int i = 0; i < parentItem->childCount(); i++)
    {
        child = parentItem->child(i);
        if(child->text(0) == name)
            return child;
    }
    child = new QTreeWidgetItem();
    parentItem->addChild(child);

    child->setText(0, name);
    addCheckBoxToItem(child);
    return child;
}

// Метод для добавления CheckBox в элемент дерева
QCheckBox* FunctionSelectorWidget::addCheckBoxToItem(QTreeWidgetItem *item)
{
    // QWidget* widget = new QWidget();
    // QHBoxLayout* layout = new QHBoxLayout(widget);
    QCheckBox *checkbox = new QCheckBox();
    checkbox->setChecked(true);
    checkbox->setTristate(false);
    // layout->addWidget(checkbox);
    // layout->setAlignment(Qt::AlignLeft);
    // layout->setContentsMargins(1, 1, 1, 1);
    // widget->setLayout(layout);
    m_treeWidget->setItemWidget(item, 1, checkbox);

    connect(checkbox, &QCheckBox::clicked, [=]() {
        if(checkbox->checkState() == Qt::PartiallyChecked) checkbox->setCheckState(Qt::Checked);
    });
    connect(checkbox, &QCheckBox::stateChanged, [=]() {
        updateParentCheckBox(item);
        updateChildCheckBoxes(item);
    });
    return checkbox;
}

void FunctionSelectorWidget::updateChildCheckBoxes(QTreeWidgetItem *item)
{
    if(!item) return;
    QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(item, 1));
    if(m_treeWidget->invisibleRootItem() == item)
        checkBox = m_checkAll;
    if(!checkBox)return;

    Qt::CheckState state = checkBox->checkState();
    if(state == Qt::PartiallyChecked) return;
    for(int i = 0; i < item->childCount(); i++)
    {
        QCheckBox *childCheckBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(item->child(i), 1));
        if(childCheckBox) childCheckBox->setCheckState(state);
    }
}

void FunctionSelectorWidget::updateParentCheckBox(QTreeWidgetItem *item)
{
    if(!item) return;
    QTreeWidgetItem *parent = item->parent();
    if(parent == nullptr){

        parent = m_treeWidget->invisibleRootItem();
        bool find = false;
        for(int i = 0; i < parent->childCount(); i++){
            if(parent->child(i) == item){
                find = true;
                break;
            }
        }
        if(!find) return;
    }
    //if(!parent) return;

    bool allChecked = true;
    bool anyChecked = false;
    for(int i = 0; i < parent->childCount(); i++)
    {
        QCheckBox *childCheckBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(parent->child(i), 1));
        if(childCheckBox)
        {
            Qt::CheckState state = childCheckBox->checkState();
            if(state == Qt::Unchecked || state == Qt::PartiallyChecked) allChecked = false;
            if(state == Qt::Checked || state == Qt::PartiallyChecked) anyChecked = true;
        }
    }

    QCheckBox *parentCheckbox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(parent, 1));
    if(parent == m_treeWidget->invisibleRootItem()){
        parentCheckbox = m_checkAll;
    }
    if(!parentCheckbox) return;
    Qt::CheckState state = Qt::Unchecked;
    if(allChecked)
        state = Qt::Checked;
    else if(anyChecked)
        state = Qt::PartiallyChecked;
    parentCheckbox->setCheckState(state);
}

void FunctionSelectorWidget::removeItem(QTreeWidgetItem *item)
{
    if(!item) return;
    if(item->childCount())
    {
        for(int i = 0; i < item->childCount(); i++)
            removeItem(item->child(i));
    }
    // удаляем checkbox
    QWidget *w = m_treeWidget->itemWidget(item, 1);
    m_treeWidget->removeItemWidget(item, 1);
    delete w;

    //удаляем ветку древа
    QTreeWidgetItem *parentItem;
    parentItem = item->parent();
    if(!parentItem)
    {
        delete m_treeWidget->takeTopLevelItem(m_treeWidget->indexOfTopLevelItem(item));
        return;
    }
    parentItem->removeChild(item);
    delete item;
}

void FunctionSelectorWidget::filterTree(const QString &text)
{
    searchTreeElement(m_treeWidget->invisibleRootItem(), text);
}

int FunctionSelectorWidget::searchTreeElement(QTreeWidgetItem *item, const QString &text)
{
    int matches = 0;
    for(int i = 0; i < item->childCount(); i++)
        matches += searchTreeElement(item->child(i), text);

    int match = item->text(0).contains(text, Qt::CaseInsensitive);
    matches += match;
    item->setHidden(!matches);
    item->setExpanded(matches);
    return matches;
}

void FunctionSelectorWidget::convertTreeWidgetToStandardItemModel(QStandardItem *parentItem, QTreeWidgetItem *treeWidgetItem)
{
    int childCount = treeWidgetItem->childCount();
    for(int i = 0; i < childCount; ++i)
    {
        QTreeWidgetItem *childTreeItem = treeWidgetItem->child(i);

        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(childTreeItem, 1));

        if(checkBox)
        {
            Qt::CheckState state = checkBox->checkState();
            QStandardItem *childStandardItem = new QStandardItem(childTreeItem->text(0));

            childStandardItem->setData(state, Qt::UserRole);
            parentItem->appendRow(childStandardItem);

            if(state == Qt::CheckState::PartiallyChecked)
            {
                // Рекурсивно добавляем потомков
                convertTreeWidgetToStandardItemModel(childStandardItem, childTreeItem);
            }
        }
    }
}

void FunctionSelectorWidget::convertTreeWidgetToVector(QVector<QPair<QString, bool>> &vector, QString func,
                                                       QTreeWidgetItem *treeWidgetItem)
{
    int childCount = treeWidgetItem->childCount();
    for(int i = 0; i < childCount; ++i)
    {
        QTreeWidgetItem *childTreeItem = treeWidgetItem->child(i);

        if(childTreeItem->childCount())
        {
            // Рекурсивно добавляем потомков
            convertTreeWidgetToVector(vector, func + childTreeItem->text(0) + "::", childTreeItem);
        }
        else
        {
            QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(childTreeItem, 1));
            if(checkBox)
            {
                QPair<QString, bool> pair;
                pair.first = func + childTreeItem->text(0);
                pair.second = checkBox->isChecked();
                vector.append(pair);
            }
        }
    }
}

void FunctionSelectorWidget::sortItems()
{
    m_treeWidget->sortItems(0, Qt::AscendingOrder);
}

QStandardItemModel *FunctionSelectorWidget::convertToStandardItemModel()
{
    QStandardItemModel *model = new QStandardItemModel();

    int topLevelCount = m_treeWidget->topLevelItemCount();
    for(int i = 0; i < topLevelCount; ++i)
    {
        QTreeWidgetItem *topLevelItem = m_treeWidget->topLevelItem(i);

        QCheckBox *checkbox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(topLevelItem, 1));

        if(checkbox)
        {
            Qt::CheckState state = checkbox->checkState();
            QStandardItem *topLevelStandardItem = new QStandardItem(topLevelItem->text(0));

            topLevelStandardItem->setData(state, Qt::UserRole);
            model->appendRow(topLevelStandardItem);

            if(state == Qt::CheckState::PartiallyChecked)
            {
                // Рекурсивно добавляем потомков
                convertTreeWidgetToStandardItemModel(topLevelStandardItem, topLevelItem);
            }
        }
    }

    return model;
}

QVector<QPair<QString, bool>> FunctionSelectorWidget::convertToVector()
{
    QVector<QPair<QString, bool>> vector;

    int topLevelCount = m_treeWidget->topLevelItemCount();
    for(int i = 0; i < topLevelCount; ++i)
    {
        QTreeWidgetItem *topLevelItem = m_treeWidget->topLevelItem(i);
        if(topLevelItem->childCount())
        {
            // Рекурсивно добавляем потомков
            convertTreeWidgetToVector(vector, topLevelItem->text(0) + "::", topLevelItem);
        }
        else
        {
            QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_treeWidget->itemWidget(topLevelItem, 1));
            if(checkBox)
            {
                QPair<QString, bool> pair;
                pair.first = topLevelItem->text(0);
                pair.second = checkBox->checkState();
                vector.append(pair);
            }
        }
    }
    return vector;
}

void FunctionSelectorWidget::updateEnablesLogMsgs()
{
    QList<QAction*> actions = m_logLevelMenu->actions();
    if(actions.size()>=5 && m_parent){
        actions[0]->setChecked(m_parent->m_settings.enableLogMsgs.infoMsg);
        actions[1]->setChecked(m_parent->m_settings.enableLogMsgs.debugMsg);
        actions[2]->setChecked(m_parent->m_settings.enableLogMsgs.warningMsg);
        actions[3]->setChecked(m_parent->m_settings.enableLogMsgs.criticalMsg);
        actions[4]->setChecked(m_parent->m_settings.enableLogMsgs.fatalMsg);
    }
}
