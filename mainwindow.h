#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QItemSelection>

class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFoodSelectionChanged(const QItemSelection &selected,
                                const QItemSelection &deselected);

private:
    bool setupDatabase();
    void initData();
    void setupModelsAndViews();
    void updateIngredientsForFood(int foodId);

    QSqlDatabase db;

    QSqlRelationalTableModel *foodModel;
    QSqlTableModel           *ingredientModel;

    QTableView *foodView;
    QTableView *ingredientView;
};

#endif // MAINWINDOW_H
