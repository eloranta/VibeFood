#include "mainwindow.h"

#include <QTableView>
#include <QHeaderView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QVariant>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    foodModel(nullptr),
    ingredientModel(nullptr),
    foodView(new QTableView(this)),
    ingredientView(new QTableView(this))
{
    setWindowTitle("VibeFood");

    if (!setupDatabase()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to open database. The app will show nothing.");
    } else {
        initData();
        setupModelsAndViews();
    }

    resize(1000, 600);
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
}

bool MainWindow::setupDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vibefood.db");

    if (!db.open()) {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // --- foods table (no categories anymore) ---
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL"
            ");")) {
        qWarning() << "Create foods error:" << query.lastError().text();
        return false;
    }

    // --- ingredients table ---
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  food_id INTEGER,"
            "  FOREIGN KEY(food_id) REFERENCES foods(id)"
            ");")) {
        qWarning() << "Create ingredients error:" << query.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::initData()
{
    QSqlQuery query(db);

    // --- Seed foods ---
    if (!query.exec("SELECT COUNT(*) FROM foods;")) {
        qWarning() << "Count foods error:" << query.lastError().text();
        return;
    }

    if (query.next() && query.value(0).toInt() == 0) {
        QSqlQuery insertFood(db);
        insertFood.prepare(
            "INSERT INTO foods (name) VALUES (:name);"
            );

        auto addFood = [&](const QString &name) {
            insertFood.bindValue(":name", name);
            if (!insertFood.exec()) {
                qWarning() << "Insert food error:" << insertFood.lastError().text();
            }
        };

        // Food list
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
    if (!query.exec("SELECT COUNT(*) FROM ingredients;")) {
        qWarning() << "Count ingredients error:" << query.lastError().text();
        return;
    }

    if (query.next() && query.value(0).toInt() == 0) {

        // Lookup food IDs by name
        QMap<QString,int> foodIds;

        QSqlQuery foodQuery(db);
        if (foodQuery.exec("SELECT id, name FROM foods;")) {
            while (foodQuery.next()) {
                foodIds.insert(foodQuery.value(1).toString(),
                               foodQuery.value(0).toInt());
            }
        }

        QSqlQuery insertIng(db);
        insertIng.prepare(
            "INSERT INTO ingredients (name, food_id) "
            "VALUES (:name, :food_id);"
            );

        auto addIng = [&](const QString &name, const QString &foodName) {
            if (!foodIds.contains(foodName)) return;
            insertIng.bindValue(":name", name);
            insertIng.bindValue(":food_id", foodIds[foodName]);
            if (!insertIng.exec()) {
                qWarning() << "Insert ingredient error:" << insertIng.lastError().text();
            }
        };

        // Multiple ingredients per food
        addIng("Tortilla wrap",  "Tortilla");
        addIng("Minced meat",    "Tortilla");
        addIng("Cheese",         "Tortilla");
        addIng("Lettuce",        "Tortilla");

        addIng("Salmon",         "Salmon soup");
        addIng("Potatoes",       "Salmon soup");
        addIng("Cream",          "Salmon soup");
        addIng("Dill",           "Salmon soup");

        addIng("Liver",          "Fried liver");
        addIng("Butter",         "Fried liver");
        addIng("Onion",          "Fried liver");

        addIng("Pasta",          "Pasta Bolognese");
        addIng("Minced meat",    "Pasta Bolognese");
        addIng("Tomato sauce",   "Pasta Bolognese");

        addIng("Salmon",         "Cured salmon");
        addIng("Salt",           "Cured salmon");
        addIng("Sugar",          "Cured salmon");
        addIng("Dill",           "Cured salmon");

        addIng("Bun",            "Hamburger");
        addIng("Beef patty",     "Hamburger");
        addIng("Cheddar",        "Hamburger");
        addIng("Pickles",        "Hamburger");

        addIng("Mushrooms",      "Fried mushrooms");
        addIng("Oil",            "Fried mushrooms");
        addIng("Garlic",         "Fried mushrooms");

        addIng("Rice",           "Salmon nigiri");
        addIng("Salmon",         "Salmon nigiri");
        addIng("Nori",           "Salmon nigiri");

        addIng("Macaroni",       "Macaroni casserole");
        addIng("Minced meat",    "Macaroni casserole");
        addIng("Cheese",         "Macaroni casserole");

        addIng("Pork ribs",      "Ribs");
        addIng("BBQ sauce",      "Ribs");

        addIng("Pulled pork",    "Pulled Pork");
        addIng("BBQ sauce",      "Pulled Pork");
        addIng("Bun",            "Pulled Pork");
    }
}

void MainWindow::setupModelsAndViews()
{
    // --- Food model ---
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    // columns: 0=id, 1=name
    foodModel->setHeaderData(0, Qt::Horizontal, "ID");
    foodModel->setHeaderData(1, Qt::Horizontal, "Food");

    if (!foodModel->select()) {
        qWarning() << "Food model select error:" << foodModel->lastError().text();
    }

    // --- Ingredient model ---
    ingredientModel = new QSqlTableModel(this, db);
    ingredientModel->setTable("ingredients");
    ingredientModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    // columns: 0=id, 1=name, 2=food_id
    ingredientModel->setHeaderData(0, Qt::Horizontal, "ID");
    ingredientModel->setHeaderData(1, Qt::Horizontal, "Ingredient");
    ingredientModel->setHeaderData(2, Qt::Horizontal, "Food ID");

    ingredientModel->select();

    // --- Configure views ---
    foodView->setModel(foodModel);
    foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    foodView->setSelectionMode(QAbstractItemView::SingleSelection);
    foodView->setEditTriggers(QAbstractItemView::DoubleClicked
                              | QAbstractItemView::SelectedClicked);
    foodView->setAlternatingRowColors(true);
    foodView->setSortingEnabled(true);
    foodView->horizontalHeader()->setStretchLastSection(true);
    foodView->setColumnHidden(0, true); // hide food ID

    ingredientView->setModel(ingredientModel);
    ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    ingredientView->setEditTriggers(QAbstractItemView::DoubleClicked
                                    | QAbstractItemView::SelectedClicked);
    ingredientView->setAlternatingRowColors(true);
    ingredientView->horizontalHeader()->setStretchLastSection(true);
    ingredientView->setColumnHidden(0, true); // hide ingredient ID
    ingredientView->setColumnHidden(2, true); // hide food_id (internal)

    // --- Bottom widget: button + ingredients view ---
    QWidget *bottomWidget = new QWidget(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *addIngredientButton = new QPushButton("Add ingredient", bottomWidget);
    bottomLayout->addWidget(addIngredientButton, 0);
    bottomLayout->addWidget(ingredientView, 1);

    // --- Splitter: foods on top, bottom widget below ---
    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(foodView);
    splitter->addWidget(bottomWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    // Connect selection -> ingredient filter
    auto *selModel = foodView->selectionModel();
    if (selModel) {
        connect(selModel, &QItemSelectionModel::selectionChanged,
                this, &MainWindow::onFoodSelectionChanged);
    }

    // Connect button
    connect(addIngredientButton, &QPushButton::clicked,
            this, &MainWindow::onAddIngredient);

    // Select first row by default
    if (foodModel->rowCount() > 0) {
        foodView->selectRow(0);
        QModelIndex idx = foodModel->index(0, 0);
        int foodId = foodModel->data(idx).toInt();
        updateIngredientsForFood(foodId);
    } else {
        updateIngredientsForFood(-1);
    }
}

void MainWindow::updateIngredientsForFood(int foodId)
{
    if (foodId < 0) {
        ingredientModel->setFilter("1 = 0");  // show nothing
    } else {
        ingredientModel->setFilter(QStringLiteral("food_id = %1").arg(foodId));
    }
    ingredientModel->select();
}

void MainWindow::onFoodSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        updateIngredientsForFood(-1);
        return;
    }

    QModelIndex anyIndex = selected.indexes().first();
    int row = anyIndex.row();

    QModelIndex idIndex = foodModel->index(row, 0); // column 0 is ID
    int foodId = foodModel->data(idIndex).toInt();

    updateIngredientsForFood(foodId);
}

void MainWindow::onAddIngredient()
{
    // Need a selected food
    auto *selModel = foodView->selectionModel();
    if (!selModel || selModel->selectedRows().isEmpty()) {
        QMessageBox::information(this, "No food selected",
                                 "Please select a food first.");
        return;
    }

    int row = selModel->selectedRows(0).first().row();
    int foodId = foodModel->data(foodModel->index(row, 0)).toInt();

    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        "Add ingredient",
        "Ingredient name:",
        QLineEdit::Normal,
        QString(),
        &ok
        );

    if (!ok || name.trimmed().isEmpty())
        return;

    QSqlQuery q(db);
    q.prepare("INSERT INTO ingredients (name, food_id) VALUES (:name, :food_id);");
    q.bindValue(":name", name.trimmed());
    q.bindValue(":food_id", foodId);
    if (!q.exec()) {
        QMessageBox::warning(this, "Insert error",
                             "Failed to add ingredient:\n" + q.lastError().text());
        return;
    }

    updateIngredientsForFood(foodId);
}
