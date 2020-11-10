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
    connect(ui->treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        qDebug() << index;
        qDebug() << _manager._mission.DebugString().data();
    });

    // connect(ui->actionNewMission, &QAction::triggered, &manager, &MissionManager::addMission);
    // connect(ui->actionNewMission, &QAction::triggered, this, [&]() { _manager.addMission(_index); });
    connect(ui->actionDelete, &QAction::triggered, this, [&]() { _manager.remove(_index); });
    connect(ui->actionAddPoint, &QAction::triggered, this, [&]() { _manager.addPoint(_index); });
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
    auto *item = _manager.model()->item(_index);
    if (item) {
        const auto &mask_action = item->backend()->action();
        if (mask_action) {
            QMenu menu(this);
            if ((mask_action >> MissionBackend::Action::kDelete) & 1) menu.addAction(ui->actionDelete);
            if (mask_action > 1) {
                QMenu *add = menu.addMenu(tr("Add"));
                if ((mask_action >> MissionBackend::Action::kAddPoint) & 1) add->addAction(ui->actionAddPoint);
                if ((mask_action >> MissionBackend::Action::kAddRail) & 1) add->addAction(ui->actionAddRail);
                if ((mask_action >> MissionBackend::Action::kAddSegment) & 1) add->addAction(ui->actionAddSegment);
                if ((mask_action >> MissionBackend::Action::kAddCollection) & 1)
                    add->addAction(ui->actionAddCollection);
            }
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    }
}
