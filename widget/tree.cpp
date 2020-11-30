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
    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
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
    connect(ui->treeView, &QTreeView::doubleClicked, this, [&](const QModelIndex &index) {
        qDebug() << index;
        auto *protobuf = index.data(Qt::UserRoleWrapper).value<MissionItem::Wrapper>().pointer;
        qDebug() << protobuf->DebugString().data();
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
    connect(ui->actionDelete, &QAction::triggered, this, [&]() {
        while (!ui->treeView->selectionModel()->selectedIndexes().isEmpty()) {
            _manager->removeIndex(ui->treeView->selectionModel()->selectedIndexes().first());
        }
    });
    connect(ui->actionAddCollection, &QAction::triggered, this,
            [&]() { _manager->addIndexCollection(ui->treeView->selectionModel()->selectedIndexes().first()); });
    connect(ui->actionAddPoint, &QAction::triggered, this,
            [&]() { _manager->addIndexPoint(ui->treeView->selectionModel()->selectedIndexes().first()); });
    connect(ui->actionAddRail, &QAction::triggered, this,
            [&]() { _manager->addIndexRail(ui->treeView->selectionModel()->selectedIndexes().first()); });
    connect(ui->actionAddSegment, &QAction::triggered, this,
            [&]() { _manager->addIndexSegment(ui->treeView->selectionModel()->selectedIndexes().first()); });
    connect(ui->actionSaveMission, &QAction::triggered, this, [&]() { _manager->saveMission(); });
    connect(ui->actionSwap, &QAction::triggered, this,
            [&]() { _manager->reverseIndexChildren(ui->treeView->selectionModel()->selectedIndexes().first()); });
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
    const auto &indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (!indexes.empty()) {

        // Compose indexes features
        auto features = _manager->model()->item(indexes.first())->supportedFeatures();
        for (const auto &index : indexes) {
            features &= _manager->model()->item(index)->supportedFeatures();
        }

        // Remove Edit feature in case of multiple selection
        if (indexes.size() > 1) features &= ~(MissionItem::kEdit);

        if (features) {
            QMenu menu(this);
            if (features & MissionItem::kDelete) menu.addAction(ui->actionDelete);
            if (features & MissionItem::kEdit) menu.addAction(ui->actionEdit);
            if (features & MissionItem::kSwap) menu.addAction(ui->actionSwap);

            if (features & (MissionItem::kCollection | MissionItem::kLine | MissionItem::kPoint)) {
                menu.addSection("Action");
                if (features & MissionItem::kCollection) menu.addAction(ui->actionAddCollection);
                if (features & MissionItem::kLine) menu.addAction(ui->actionAddRail);
                if (features & MissionItem::kLine) menu.addAction(ui->actionAddSegment);
                if (features & MissionItem::kPoint) menu.addAction(ui->actionAddPoint);
            }

            // When only one index is selected and this index is the top mission index.
            if (indexes.size() == 1 &&
                MissionItem::Features(indexes.first().data(Qt::UserRoleFeature).toInt() & MissionItem::kMission)) {
                menu.addSection("Mission");
                menu.addAction(ui->actionNewMission);
                menu.addAction(ui->actionOpenMission);
                menu.addAction(ui->actionSaveMissionAs);
                if (_manager->canSaveMission()) {
                    menu.addAction(ui->actionSaveMission);
                }
            }
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    } else {
        // Add new mission if the root item has no child.
        if (!_manager->model()->root()->countChild()) {
            QMenu menu(this);
            menu.addSection("Mission");
            menu.addAction(ui->actionNewMission);
            menu.addAction(ui->actionOpenMission);
            menu.exec(ui->treeView->viewport()->mapToGlobal(position));
        }
    }
}
