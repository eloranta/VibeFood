#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

class QSqlTableModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *foodModel;
    QSqlTableModel *ingredientsModel;

    bool openDatabase();
    bool createTables();
    void seedDataIfEmpty();
    void setupModelAndView();

};
#endif // MAINWINDOW_H
