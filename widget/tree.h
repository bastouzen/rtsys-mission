#ifndef MISSION_TREE_WIDGET_H
#define MISSION_TREE_WIDGET_H

#include <QModelIndex>
#include <QWidget>

class MissionManager;

namespace Ui {
class MissionTreeWidget;
}

class MissionTreeWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit MissionTreeWidget(QWidget *parent = nullptr);
    ~MissionTreeWidget();

    void setManager(MissionManager *manager);

  private:
    void createCustomContexMenu(const QPoint &position);
    Ui::MissionTreeWidget *ui;
    MissionManager *_manager;
};

#endif // MISSION_TREE_WIDGET_H
