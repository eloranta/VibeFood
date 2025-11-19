#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>

class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool setupDatabase();
    void initData();
    void setupModelAndView();

    QSqlDatabase db;
    QSqlRelationalTableModel *model;
    QTableView *tableView;
};

#endif // MAINWINDOW_H
