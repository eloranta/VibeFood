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

class IngredientModelWithCheck;

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
    IngredientModelWithCheck *ingredientsModel;

    bool openDatabase();
    bool createTables();
    void seedDataIfEmpty();
    void setupModelAndView();
    void addItem(const QString &food, const QString &ingredient, const QString &amount);

private slots:
    void onFoodSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onShowAllToggled(bool checked);
    void onRecipeChanged();
    void onAddFoodClicked();
};

#endif // MAINWINDOW_H
