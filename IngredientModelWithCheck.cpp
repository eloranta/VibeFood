#include "IngredientModelWithCheck.h"
#include <QSqlQuery>
#include <QDebug>

IngredientModelWithCheck::IngredientModelWithCheck(QObject *parent,
                                                   const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
}

int IngredientModelWithCheck::columnCount(const QModelIndex &parent) const
{
    // +2: checkbox column + amount column
    return QSqlTableModel::columnCount(parent) + 2;
}

bool IngredientModelWithCheck::select()
{
    bool ok = QSqlTableModel::select();

    int rows = rowCount();
    m_checks  = QVector<Qt::CheckState>(rows, Qt::Unchecked);
    m_amounts = QVector<QString>(rows, "");

    return ok;
}

QVariant IngredientModelWithCheck::data(const QModelIndex &index,
                                        int role) const
{
    if (!index.isValid())
        return QVariant();

    int r = index.row();
    int c = index.column();

    // ----- Column 0: checkbox -----
    if (c == 0)
    {
        if (role == Qt::CheckStateRole)
            return m_checks[r];
        return QVariant();
    }

    // ----- Column 3: amount (virtual) -----
    if (c == 3)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return m_amounts[r];
        return QVariant();
    }

    // SQL-backed columns shift by -1
    QModelIndex src = QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::data(src, role);
}

bool IngredientModelWithCheck::setData(const QModelIndex &index,
                                       const QVariant &value,
                                       int role)
{
    if (!index.isValid())
        return false;

    int r = index.row();
    int c = index.column();

    // ----- CHECKBOX COLUMN -----
    if (c == 0 && role == Qt::CheckStateRole)
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        m_checks[r] = state;

        // If unchecked → clear amount
        if (state == Qt::Unchecked)
            m_amounts[r].clear();

        emit dataChanged(index, index, {Qt::CheckStateRole});

        int ingredientId =
            QSqlTableModel::data(QSqlTableModel::index(r, 0),
                                 Qt::DisplayRole).toInt();

        if (m_currentFoodId >= 0)
        {
            QSqlQuery q(database());

            if (state == Qt::Checked)
            {
                q.prepare("INSERT OR IGNORE INTO food_ingredients "
                          "(food_id, ingredient_id, amount) "
                          "VALUES (:f, :i, '')");
                q.bindValue(":f", m_currentFoodId);
                q.bindValue(":i", ingredientId);
                q.exec();
            }
            else
            {
                q.prepare("DELETE FROM food_ingredients "
                          "WHERE food_id = :f AND ingredient_id = :i");
                q.bindValue(":f", m_currentFoodId);
                q.bindValue(":i", ingredientId);
                q.exec();
            }
        }

        return true;
    }

    // ----- AMOUNT COLUMN -----
    if (c == 3 && role == Qt::EditRole)
    {
        // Only editable if checkbox is checked
        if (m_checks[r] == Qt::Unchecked)
            return false;

        QString newAmount = value.toString();
        m_amounts[r] = newAmount;

        emit dataChanged(index, index);

        int ingredientId =
            QSqlTableModel::data(QSqlTableModel::index(r, 0),
                                 Qt::DisplayRole).toInt();

        if (m_currentFoodId >= 0)
        {
            QSqlQuery q(database());
            q.prepare("UPDATE food_ingredients "
                      "SET amount = :a "
                      "WHERE food_id = :f AND ingredient_id = :i");
            q.bindValue(":a", newAmount);
            q.bindValue(":f", m_currentFoodId);
            q.bindValue(":i", ingredientId);
            q.exec();
        }

        return true;
    }

    // ----- DEFAULT (SQL FIELDS) -----
    QModelIndex src = QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::setData(src, value, role);
}

Qt::ItemFlags IngredientModelWithCheck::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    int c = index.column();

    // Checkbox column
    if (c == 0)
        return Qt::ItemIsEnabled |
               Qt::ItemIsSelectable |
               Qt::ItemIsUserCheckable;

    // Amount column editable only when checked
    if (c == 3)
    {
        if (m_checks[index.row()] == Qt::Checked)
            return Qt::ItemIsEnabled |
                   Qt::ItemIsEditable |
                   Qt::ItemIsSelectable;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    // Other SQL columns
    QModelIndex src = QSqlTableModel::index(index.row(), c - 1);
    return QSqlTableModel::flags(src);
}

QVariant IngredientModelWithCheck::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0) return QString("✔");
        if (section == 3) return QString("Amount");
        return QSqlTableModel::headerData(section - 1, orientation, role);
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

void IngredientModelWithCheck::setCurrentFoodId(int id)
{
    m_currentFoodId = id;
}

void IngredientModelWithCheck::setCheckedRows(
    const QSet<int> &ingredientIds,
    const QMap<int, QString> &amounts)
{
    for (int r = 0; r < rowCount(); ++r)
    {
        int ingredientId =
            QSqlTableModel::data(QSqlTableModel::index(r, 0),
                                 Qt::DisplayRole).toInt();

        if (ingredientIds.contains(ingredientId))
        {
            m_checks[r]  = Qt::Checked;
            m_amounts[r] = amounts.value(ingredientId);
        }
        else
        {
            m_checks[r]  = Qt::Unchecked;
            m_amounts[r] = "";
        }
    }

    emit dataChanged(index(0,0),
                     index(rowCount()-1, 3),
                     {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole});
}
