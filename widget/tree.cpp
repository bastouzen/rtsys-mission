#include "ui_tree.h"

#include "mission/item.h"
#include "mission/manager.h"
#include "widget/tree.h"

#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QStandardPaths>

MissionTreeWidget::MissionTreeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MissionTreeWidget)
    , _manager(nullptr)
{
    ui->setupUi(this);
    // ui->treeView->setModel(_manager.model());

    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    // ui->treeView->setSelectionModel()

    // Enable right-click context.
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MissionTreeWidget::createCustomContexMenu);

    // Enable drag&drop.
    ui->treeView->setDragEnabled(true);
    ui->treeView->viewport()->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    //    ui->treeView->setDragDropMode(QAbstractItemView::InternalMove);

    // connect(ui->actionSelectAll, &QAction::triggered, ui->treeView, &QTreeView::selectAll);
    connect(ui->treeView, &QTreeView::doubleClicked, this, [](const QModelIndex &index) {
        // qDebug() << index;
        // qDebug() << _manager._mission.DebugString().data();
    });
}

MissionTreeWidget::~MissionTreeWidget()
{
    delete ui;
}

void MissionTreeWidget::setManager(MissionManager *manager)
{
    _manager = manager;
    ui->treeView->setModel(_manager->model());
    connect(ui->actionNewMission, &QAction::triggered, _manager, &MissionManager::newMission);
    connect(ui->actionDelete, &QAction::triggered, this, [&]() { _manager->removeIndex(_index); });
    connect(ui->actionAddCollection, &QAction::triggered, this, [&]() { _manager->addCollectionIndex(_index); });
    connect(ui->actionAddPoint, &QAction::triggered, this, [&]() { _manager->addPointIndex(_index); });
    connect(ui->actionAddRail, &QAction::triggered, this, [&]() { _manager->addRailIndex(_index); });
    connect(ui->actionAddSegment, &QAction::triggered, this, [&]() { _manager->addSegmentIndex(_index); });
    connect(ui->actionSaveMission, &QAction::triggered, this, [&]() { _manager->saveMission(); });
    connect(ui->actionSaveMissionAs, &QAction::triggered, this, [&]() {
        _manager->saveMissionAs(QFileDialog::getSaveFileName(
            this, tr("Save Mission File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            tr("JSON (*.json)")));
    });
    connect(ui->actionOpenMission, &QAction::triggered, this, [&]() {
        _manager->openMission(QFileDialog::getOpenFileName(
            this, tr("Open Mission File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            tr("JSON (*.json)")));
    });
    connect(_manager, &MissionManager::loadMissionDone, ui->treeView, [this]() {
        ui->treeView->expandAll();
        ui->treeView->resizeColumnToContents(0);
    });
}

void MissionTreeWidget::createCustomContexMenu(const QPoint &position)
{
    _index = ui->treeView->indexAt(position);

    auto *item = _manager->model()->item(_index);

    if (item) {
        auto &backend = item->backend();
        const auto &mask_action = backend.authorization();
        if (mask_action) {
            QMenu menu(this);
            if (backend.isAuthorized(ModelBacken::kDelete, mask_action)) menu.addAction(ui->actionDelete);
            if (mask_action > 1) {
                QMenu *add = menu.addMenu(tr("Add"));
                if (backend.isAuthorized(ModelBacken::kPoint, mask_action)) add->addAction(ui->actionAddPoint);
                if (backend.isAuthorized(ModelBacken::kRail, mask_action)) add->addAction(ui->actionAddRail);
                if (backend.isAuthorized(ModelBacken::kSegment, mask_action)) add->addAction(ui->actionAddSegment);
                if (backend.isAuthorized(ModelBacken::kCollection, mask_action))
                    add->addAction(ui->actionAddCollection);
            }

            if (backend.component() == ModelBacken::kMission) {
                menu.addAction(ui->actionNewMission);
                menu.addAction(ui->actionOpenMission);
                menu.addAction(ui->actionSaveMission);
                menu.addAction(ui->actionSaveMissionAs);
            }
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    } else {
        // Add new mission if the root item has no child.
        if (!_manager->model()->root()->childCount()) {
            QMenu menu(this);
            QMenu *add = menu.addMenu(tr("Add"));
            add->addAction(ui->actionNewMission);
            add->addAction(ui->actionOpenMission);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    }
}
