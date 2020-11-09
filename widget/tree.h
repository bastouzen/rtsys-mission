#ifndef MISSION_TREE_WIDGET_H
#define MISSION_TREE_WIDGET_H

#include "manager.h"

#include <QWidget>

namespace Ui {
class MissionTreeWidget;
}

class MissionTreeWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit MissionTreeWidget(QWidget *parent = nullptr);
    ~MissionTreeWidget();

    void loadMission(pb::mission::Mission *mission);

  private:
    void createCustomContexMenu(const QPoint &position);
    Ui::MissionTreeWidget *ui;
    MissionManager _manager;
    QModelIndex _index;
};

#endif // MISSION_TREE_WIDGET_H
