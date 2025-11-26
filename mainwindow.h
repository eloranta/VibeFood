#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool openDatabase();
    bool createTables();
    void addItem(const QString &food, const QString &ingredient, const QString &amount);
    void seedDataIfEmpty();
    void setupModelAndView();
    void setIngredientFilterForFood(int foodId);
    void addFood();
    void deleteFood();
    void addIngredient();
    void deleteIngredient();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *foodModel;
    QSqlTableModel *ingredientModel;
    bool updatingRecipeText = false;
};

#endif // MAINWINDOW_H
