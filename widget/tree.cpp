#include "ui_tree.h"

//#include "private/backend.h"
#include "private/model.h"
#include "widget/tree.h"

#include <QDebug>
#include <QMenu>

MissionTreeWidget::MissionTreeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MissionTreeWidget)
{
    ui->setupUi(this);
    ui->treeView->setModel(_manager.model());
    // ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    // ui->treeView->setSelectionModel()

    // Enable right-click context.
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MissionTreeWidget::createCustomContexMenu);

    // connect(ui->actionSelectAll, &QAction::triggered, ui->treeView, &QTreeView::selectAll);
    connect(ui->treeView, &QTreeView::doubleClicked, this, [](const QModelIndex &index) { qDebug() << index; });

    // connect(ui->actionNewMission, &QAction::triggered, &manager, &MissionManager::addMission);
    connect(ui->actionNewMission, &QAction::triggered, this, [&]() { _manager.addMission(_index); });
    connect(ui->actionDelete, &QAction::triggered, this, [&]() { _manager.deleteS(_index); });
}

MissionTreeWidget::~MissionTreeWidget()
{
    delete ui;
}

void MissionTreeWidget::loadMission(pb::mission::Mission *mission)
{
    _manager.loadMission(mission);
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
}

void MissionTreeWidget::createCustomContexMenu(const QPoint &position)
{
    _index = ui->treeView->indexAt(position);
    qDebug() << "createCustomContexMenu" << _index << _index.isValid() << _index.parent() << _index.parent().isValid();

    auto *item = _manager.model()->item(_index);
    if (item) {
        const auto &component_type = item->backend()->componentType();

        if (component_type == MissionBackend::kMission) {
            QMenu menu(this);
            menu.addAction(ui->actionDelete);
            QMenu *add = menu.addMenu(tr("Add"));
            add->addAction(ui->actionAddPoint);
            add->addAction(ui->actionAddRail);
            add->addAction(ui->actionAddSegment);
            add->addAction(ui->actionAddCollection);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));

        } else if (component_type == MissionBackend::kCollection) {
            QMenu menu(this);
            menu.addAction(ui->actionDelete);
            QMenu *add = menu.addMenu(tr("Add"));
            add->addAction(ui->actionAddPoint);
            add->addAction(ui->actionAddRail);
            add->addAction(ui->actionAddSegment);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));

        } else if (component_type == MissionBackend::kPoint) {
            const auto &parent_component_type = item->parent()->backend()->componentType();
            if (parent_component_type != MissionBackend::kRail && parent_component_type != MissionBackend::kSegment) {
                QMenu menu(this);
                menu.addAction(ui->actionDelete);
                menu.exec(ui->treeView->viewport()->mapToGlobal(position));
            }

        } else if (component_type == MissionBackend::kRail || component_type == MissionBackend::kSegment) {
            QMenu menu(this);
            menu.addAction(ui->actionDelete);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }

    } else {
        // If the modem item is invalid and the modem root item has no child
        if (_manager.model()->root()->childCount() == 0) {
            QMenu menu(this);
            menu.addAction(ui->actionNewMission);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    }
}
