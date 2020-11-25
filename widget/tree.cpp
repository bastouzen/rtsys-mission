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
    connect(ui->actionDelete, &QAction::triggered, this, [&]() { _manager->removeIndex(_index); });
    connect(ui->actionAddCollection, &QAction::triggered, this, [&]() { _manager->addIndexCollection(_index); });
    connect(ui->actionAddPoint, &QAction::triggered, this, [&]() { _manager->addIndexPoint(_index); });
    connect(ui->actionAddRail, &QAction::triggered, this, [&]() { _manager->addIndexRail(_index); });
    connect(ui->actionAddSegment, &QAction::triggered, this, [&]() { _manager->addIndexSegment(_index); });
    connect(ui->actionSaveMission, &QAction::triggered, this, [&]() { _manager->saveMission(); });
    connect(ui->actionSwap, &QAction::triggered, this, [&]() { _manager->swapIndex(_index); });
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
        const auto &flags = item->supportedFlags();
        if (flags) {
            QMenu menu(this);

            if (item->isFlagSupported(MissionItem::kDelete, flags)) menu.addAction(ui->actionDelete);
            if (item->isFlagSupported(MissionItem::kEdit, flags)) menu.addAction(ui->actionEdit);
            if (item->isFlagSupported(MissionItem::kSwap, flags)) menu.addAction(ui->actionSwap);

            menu.addSection("Action");
            if (item->isFlagSupported(MissionItem::kPoint, flags)) menu.addAction(ui->actionAddPoint);
            if (item->isFlagSupported(MissionItem::kRail, flags)) menu.addAction(ui->actionAddRail);
            if (item->isFlagSupported(MissionItem::kSegment, flags)) menu.addAction(ui->actionAddSegment);
            if (item->isFlagSupported(MissionItem::kCollection, flags)) menu.addAction(ui->actionAddCollection);

            if (item->data(Qt::UserRoleFlag) == MissionItem::kMission) {
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
