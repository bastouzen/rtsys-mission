#include "ui_tree.h"

#include "mission/manager.h"
#include "mission/item.h"
#include "widget/tree.h"

#include <QDebug>
#include <QMenu>

MissionTreeWidget::MissionTreeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MissionTreeWidget)
    , _manager(nullptr)
{
    ui->setupUi(this);
    // ui->treeView->setModel(_manager.model());

    // ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    // ui->treeView->setSelectionModel()

    // Enable right-click context.
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MissionTreeWidget::createCustomContexMenu);

    // connect(ui->actionSelectAll, &QAction::triggered, ui->treeView, &QTreeView::selectAll);
    connect(ui->treeView, &QTreeView::doubleClicked, this, [](const QModelIndex &index) {
        qDebug() << index;
        // qDebug() << _manager._mission.DebugString().data();
    });

    // connect(ui->actionNewMission, &QAction::triggered, &manager, &MissionManager::addMission);
    // connect(ui->actionNewMission, &QAction::triggered, this, [&]() { _manager.addMission(_index); });
}

MissionTreeWidget::~MissionTreeWidget()
{
    delete ui;
}

void MissionTreeWidget::setManager(MissionManager *manager)
{
    _manager = manager;
    ui->treeView->setModel(_manager->model());
    //    connect(ui->actionDelete, &QAction::triggered, this, [&]() { _manager->remove(_index); });
    //    connect(ui->actionAddPoint, &QAction::triggered, this, [&]() { _manager->addPoint(_index); });
    //    connect(ui->actionAddRail, &QAction::triggered, this, [&]() { _manager->addRail(_index); });
}

// void MissionTreeWidget::loadMission(const pb::mission::Mission &mission)
//{
//    _manager.loadMission(mission);
//    ui->treeView->expandAll();
//    ui->treeView->resizeColumnToContents(0);
//}

void MissionTreeWidget::createCustomContexMenu(const QPoint &position)
{
    _index = ui->treeView->indexAt(position);
    auto *item = _manager->model()->item(_index);
    if (item) {
        auto &backend = item->backend();
        const auto &mask_action = backend.maskEnableAction();
        if (mask_action) {
            QMenu menu(this);
            if (backend.hasEnableAction(MissionBackend::Action::kDelete, mask_action)) menu.addAction(ui->actionDelete);
            if (mask_action > 1) {
                QMenu *add = menu.addMenu(tr("Add"));
                if (backend.hasEnableAction(MissionBackend::Action::kAddPoint, mask_action))
                    add->addAction(ui->actionAddPoint);
                if (backend.hasEnableAction(MissionBackend::Action::kAddRail, mask_action))
                    add->addAction(ui->actionAddRail);
                if (backend.hasEnableAction(MissionBackend::Action::kAddSegment, mask_action))
                    add->addAction(ui->actionAddSegment);
                if (backend.hasEnableAction(MissionBackend::Action::kAddCollection, mask_action))
                    add->addAction(ui->actionAddCollection);
            }
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    }
}
