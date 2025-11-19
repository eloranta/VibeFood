#include <QDir>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (!openDatabase())
    {
        QMessageBox::critical(this, "Database error", "Failed to open SQLite database.");
        return;
    }

    if (!createTables())
    {
        QMessageBox::critical(this, "Database error", "Failed to create tables.");
        return;
    }

    seedDataIfEmpty();

    setupModelAndView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openDatabase()
{
    // Put the DB file next to the executable
    QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("vibefood.db");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}

bool MainWindow::createTables()
{
    QSqlQuery q(db);

    // foods: food_id, name
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  food_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create foods error:" << q.lastError().text();
        return false;
    }

    // ingredients: ingredient_id, name
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  ingredient_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create ingredients error:" << q.lastError().text();
        return false;
    }

    // pivot: which ingredients each food contains, with optional amount
    // if (!q.exec(
    //         "CREATE TABLE IF NOT EXISTS food_ingredients ("
    //         "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    //         "  food_id INTEGER NOT NULL,"
    //         "  ingredient_id INTEGER NOT NULL,"
    //         "  amount TEXT,"
    //         "  FOREIGN KEY(food_id) REFERENCES foods(food_id),"
    //         "  FOREIGN KEY(ingredient_id) REFERENCES ingredients(ingredient_id)"
    //         ");")) {
    //     qWarning() << "Create food_ingredients error:" << q.lastError().text();
    //     return false;
    // }

    return true;
}

void MainWindow::seedDataIfEmpty()
{
    QSqlQuery q(db);

    // --- Seed foods ---
    if (!q.exec("SELECT COUNT(*) FROM foods;"))
    {
        qWarning() << "Count foods error:" << q.lastError().text();
        return;
    }
    bool needFoods = false;
    if (q.next() && q.value(0).toInt() == 0)
        needFoods = true;

    if (needFoods)
    {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO foods (name) VALUES (:name);");

        auto addFood = [&](const QString &name)
        {
            ins.bindValue(":name", name);
            if (!ins.exec())
            {
                qWarning() << "Insert food error:" << ins.lastError().text();
            }
        };

        addFood("Tortilla");
        addFood("Salmon soup");
        addFood("Fried liver");
        addFood("Pasta Bolognese");
        addFood("Cured salmon");
        addFood("Hamburger");
        addFood("Fried mushrooms");
        addFood("Salmon nigiri");
        addFood("Macaroni casserole");
        addFood("Ribs");
        addFood("Pulled Pork");
    }

    // --- Seed ingredients ---
    if (!q.exec("SELECT COUNT(*) FROM ingredients;"))
    {
        qWarning() << "Count ingredients error:" << q.lastError().text();
        return;
    }
    bool needIngredients = false;
    if (q.next() && q.value(0).toInt() == 0)
        needIngredients = true;

    if (needIngredients)
    {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO ingredients (name) VALUES (:name);");

        auto addIng = [&](const QString &name) {
            ins.bindValue(":name", name);
            if (!ins.exec()) {
                qWarning() << "Insert ingredient error:" << ins.lastError().text();
            }
        };

        addIng("Tortilla wrap");
        addIng("Minced meat");
        addIng("Cheese");
        addIng("Lettuce");
        addIng("Salmon");
        addIng("Potatoes");
        addIng("Cream");
        addIng("Dill");
        addIng("Liver");
        addIng("Butter");
        addIng("Onion");
        addIng("Pasta");
        addIng("Tomato sauce");
        addIng("Salt");
        addIng("Sugar");
        addIng("Bun");
        addIng("Beef patty");
        addIng("Cheddar");
        addIng("Pickles");
        addIng("Mushrooms");
        addIng("Oil");
        addIng("Garlic");
        addIng("Rice");
        addIng("Nori");
        addIng("Macaroni");
        addIng("BBQ sauce");
    }

 /*   // --- Seed pivot table ---
    if (!q.exec("SELECT COUNT(*) FROM food_ingredients;")) {
        qWarning() << "Count food_ingredients error:" << q.lastError().text();
        return;
    }
    bool needPivot = false;
    if (q.next() && q.value(0).toInt() == 0)
        needPivot = true;

    if (!needPivot)
        return;

    // Build name -> id maps
    QMap<QString, int> foodIdByName;
    QMap<QString, int> ingIdByName;

    QSqlQuery fq(db);
    if (fq.exec("SELECT food_id, name FROM foods;")) {
        while (fq.next()) {
            foodIdByName[fq.value(1).toString()] = fq.value(0).toInt();
        }
    }

    QSqlQuery iq(db);
    if (iq.exec("SELECT ingredient_id, name FROM ingredients;")) {
        while (iq.next()) {
            ingIdByName[iq.value(1).toString()] = iq.value(0).toInt();
        }
    }

    auto foodId = [&](const QString &name) -> int {
        return foodIdByName.value(name, -1);
    };
    auto ingId = [&](const QString &name) -> int {
        return ingIdByName.value(name, -1);
    };

    QSqlQuery ins(db);
    ins.prepare(
        "INSERT INTO food_ingredients (food_id, ingredient_id, amount) "
        "VALUES (:food_id, :ingredient_id, :amount);");

    auto addLink = [&](const QString &foodName,
                       const QString &ingName,
                       const QString &amount) {
        int fId = foodId(foodName);
        int iId = ingId(ingName);
        if (fId <= 0 || iId <= 0)
            return;

        ins.bindValue(":food_id", fId);
        ins.bindValue(":ingredient_id", iId);
        ins.bindValue(":amount", amount);
        if (!ins.exec()) {
            qWarning() << "Insert food_ingredients error:"
                       << ins.lastError().text();
        }
    };

    // Example compositions
    addLink("Tortilla", "Tortilla wrap", "1 pc");
    addLink("Tortilla", "Minced meat", "150 g");
    addLink("Tortilla", "Cheese", "50 g");
    addLink("Tortilla", "Lettuce", "some");

    addLink("Salmon soup", "Salmon", "150 g");
    addLink("Salmon soup", "Potatoes", "3 pcs");
    addLink("Salmon soup", "Cream", "200 ml");
    addLink("Salmon soup", "Dill", "to taste");

    addLink("Hamburger", "Bun", "1 pc");
    addLink("Hamburger", "Beef patty", "1 pc");
    addLink("Hamburger", "Cheddar", "1 slice");
    addLink("Hamburger", "Pickles", "few slices");

    addLink("Salmon nigiri", "Rice", "100 g");
    addLink("Salmon nigiri", "Salmon", "2 slices");
    addLink("Salmon nigiri", "Nori", "optional");

    addLink("Ribs", "Pork ribs", "300 g");
    addLink("Ribs", "BBQ sauce", "50 g");

    addLink("Pulled Pork", "Pulled pork", "150 g");
    addLink("Pulled Pork", "BBQ sauce", "30 g");
    addLink("Pulled Pork", "Bun", "1 pc");
*/
}

void MainWindow::setupModelAndView()
{
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!foodModel->select())
    {
        qWarning() << "food select error:" << foodModel->lastError().text();
    }

    ingredientsModel = new QSqlTableModel(this, db);
    ingredientsModel->setTable("foods");
    ingredientsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!ingredientsModel->select())
    {
        qWarning() << "ingredients select error:" << ingredientsModel->lastError().text();
    }

    ui->foodView->setModel(foodModel);
//    foodView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->foodView->setSelectionMode(QAbstractItemView::SingleSelection);
//    foodView->setSortingEnabled(true);

    ui->ingredientView->setModel(foodModel);
    //    ingredientView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    //    ingredientView->setSortingEnabled(true);

}



