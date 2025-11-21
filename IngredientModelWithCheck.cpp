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
    return QSqlTableModel::columnCount(parent) + 1; // +1 for checkbox column
}

bool IngredientModelWithCheck::select()
{
    bool ok = QSqlTableModel::select();

    // Resize checkbox list to match rows
    m_checks = QVector<Qt::CheckState>(rowCount(), Qt::Unchecked);

    return ok;
}

QVariant IngredientModelWithCheck::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    // Virtual checkbox column
    if (col == 0)
    {
        if (role == Qt::CheckStateRole)
            return m_checks[row];

        // No text in checkbox column
        return QVariant();
    }

    // Real SQL columns shifted by -1
    QModelIndex src = QSqlTableModel::index(row, col - 1);
    return QSqlTableModel::data(src, role);
}

bool IngredientModelWithCheck::setData(const QModelIndex &index,
                                       const QVariant &value,
                                       int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    // --- CHECKBOX COLUMN ---
    if (col == 0 && role == Qt::CheckStateRole)
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        m_checks[row] = state;

        emit dataChanged(index, index, {Qt::CheckStateRole});

        // DB sync
        if (m_currentFoodId >= 0)
        {
            int ingredientId =
                QSqlTableModel::data(QSqlTableModel::index(row, 0), Qt::DisplayRole).toInt();

            QSqlQuery q(database());

            if (state == Qt::Checked)
            {
                q.prepare("INSERT OR IGNORE INTO food_ingredients "
                          "(food_id, ingredient_id) VALUES (:f, :i)");
                q.bindValue(":f", m_currentFoodId);
                q.bindValue(":i", ingredientId);
                q.exec();
                qDebug() << "INSERT into pivot" << m_currentFoodId << ingredientId;
            }
            else
            {
                q.prepare("DELETE FROM food_ingredients "
                          "WHERE food_id = :f AND ingredient_id = :i");
                q.bindValue(":f", m_currentFoodId);
                q.bindValue(":i", ingredientId);
                q.exec();
                qDebug() << "DELETE from pivot" << m_currentFoodId << ingredientId;
            }
        }

        return true;
    }

    // Other SQL-backed columns
    QModelIndex src = QSqlTableModel::index(row, col - 1);
    return QSqlTableModel::setData(src, value, role);
}

Qt::ItemFlags IngredientModelWithCheck::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 0)
    {
        return Qt::ItemIsEnabled |
               Qt::ItemIsUserCheckable |
               Qt::ItemIsSelectable;
    }

    QModelIndex src = QSqlTableModel::index(index.row(), index.column() - 1);
    return QSqlTableModel::flags(src);
}

QVariant IngredientModelWithCheck::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (section == 0 && role == Qt::DisplayRole)
            return QString(""); // Checkbox column header empty

        return QSqlTableModel::headerData(section - 1, orientation, role);
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

void IngredientModelWithCheck::setCurrentFoodId(int id)
{
    m_currentFoodId = id;
}

void IngredientModelWithCheck::setCheckedRows(const QSet<int> &ingredientIds)
{
    for (int row = 0; row < rowCount(); ++row)
    {
        int ingredientId =
            QSqlTableModel::data(QSqlTableModel::index(row, 0), Qt::DisplayRole).toInt();

        m_checks[row] = ingredientIds.contains(ingredientId)
                            ? Qt::Checked
                            : Qt::Unchecked;
    }

    emit dataChanged(index(0,0),
                     index(rowCount()-1, 0),
                     {Qt::CheckStateRole});
}
