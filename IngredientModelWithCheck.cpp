#include "IngredientModelWithCheck.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

IngredientModelWithCheck::IngredientModelWithCheck(
    QObject *parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
}

// -------------------------------
// Column count
// -------------------------------
int IngredientModelWithCheck::columnCount(const QModelIndex &parent) const
{
    return QSqlTableModel::columnCount(parent) + 2; // checkbox + amount
}

// -------------------------------
// Helper: ingredient_id for a given view row
// -------------------------------
int IngredientModelWithCheck::ingredientIdForRow(int row) const
{
    return QSqlTableModel::data(
               QSqlTableModel::index(row, 0),
               Qt::DisplayRole
               ).toInt();
}

// -------------------------------
// SELECT → reset check/amount maps
// -------------------------------
bool IngredientModelWithCheck::select()
{
    bool ok = QSqlTableModel::select();

    m_checks.clear();
    m_amounts.clear();

    return ok;
}

// -------------------------------
// DATA
// -------------------------------
QVariant IngredientModelWithCheck::data(
    const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    int ingredientId = ingredientIdForRow(row);

    // Column 0 = checkbox
    if (col == 0)
    {
        if (role == Qt::CheckStateRole)
            return m_checks.value(ingredientId, Qt::Unchecked);
        return QVariant();
    }

    // Column 3 = amount
    if (col == 3)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return m_amounts.value(ingredientId);
        return QVariant();
    }

    // SQL-backed columns (shift by 1)
    return QSqlTableModel::data(
        QSqlTableModel::index(row, col - 1), role);
}

// -------------------------------
// SET DATA
// -------------------------------
bool IngredientModelWithCheck::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();
    int ingredientId = ingredientIdForRow(row);

    // ----------------------------------------
    // CHECKBOX
    // ----------------------------------------
    if (col == 0 && role == Qt::CheckStateRole)
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        m_checks[ingredientId] = state;

        if (state == Qt::Unchecked)
            m_amounts.remove(ingredientId);

        emit dataChanged(index, index, {Qt::CheckStateRole});

        // Pivot update
        if (m_currentFoodId >= 0)
        {
            QSqlQuery q(database());
            if (state == Qt::Checked)
            {
                q.prepare("INSERT OR IGNORE INTO food_ingredients "
                          "(food_id, ingredient_id, amount) "
                          "VALUES (:f, :i, '')");
            }
            else
            {
                q.prepare("DELETE FROM food_ingredients "
                          "WHERE food_id = :f AND ingredient_id = :i");
            }

            q.bindValue(":f", m_currentFoodId);
            q.bindValue(":i", ingredientId);
            q.exec();
        }
        return true;
    }

    // ----------------------------------------
    // AMOUNT COLUMN
    // ----------------------------------------
    if (col == 3 && role == Qt::EditRole)
    {
        if (m_checks.value(ingredientId) != Qt::Checked)
            return false;

        m_amounts[ingredientId] = value.toString();

        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        // Pivot update
        if (m_currentFoodId >= 0)
        {
            QSqlQuery q(database());
            q.prepare("UPDATE food_ingredients "
                      "SET amount = :a "
                      "WHERE food_id = :f AND ingredient_id = :i");
            q.bindValue(":a", value.toString());
            q.bindValue(":f", m_currentFoodId);
            q.bindValue(":i", ingredientId);
            q.exec();
        }

        return true;
    }

    // ----------------------------------------
    // NAME column (SQL col 1)
    // ----------------------------------------
    if (col == 2 && role == Qt::EditRole)
    {
        QModelIndex sqlIndex = QSqlTableModel::index(row, 1);

        bool ok = QSqlTableModel::setData(sqlIndex, value, role);

        if (!ok)
            return false;

        if (!QSqlTableModel::submitAll())
            qWarning() << "submitAll failed:" << lastError().text();

        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }

    // ID column not editable
    return false;
}

// -------------------------------
// FLAGS
// -------------------------------
Qt::ItemFlags IngredientModelWithCheck::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    int col = index.column();
    int row = index.row();
    int ingredientId = ingredientIdForRow(row);

    if (col == 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;

    if (col == 1)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // id

    if (col == 2)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; // name

    if (col == 3)
    {
        if (m_checks.value(ingredientId) == Qt::Checked)
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// -------------------------------
// HEADERS
// -------------------------------
QVariant IngredientModelWithCheck::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return "✔";
        case 1: return "ID";
        case 2: return "Ingredient";
        case 3: return "Amount";
        }
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

// -------------------------------
// SET CURRENT FOOD
// -------------------------------
void IngredientModelWithCheck::setCurrentFoodId(int id)
{
    m_currentFoodId = id;
}

// -------------------------------
// LOAD CHECKS + AMOUNTS FROM DB
// -------------------------------
void IngredientModelWithCheck::setCheckedRows(
    const QSet<int> &ingredientIds,
    const QMap<int, QString> &amounts)
{
    m_checks.clear();
    m_amounts.clear();

    for (int id : ingredientIds)
        m_checks[id] = Qt::Checked;

    m_amounts = amounts;

    if (rowCount() > 0)
        emit dataChanged(index(0,0), index(rowCount()-1,3));
}
