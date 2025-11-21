#include <QSqlQuery>
#include "IngredientModelWithCheck.h"

IngredientModelWithCheck::IngredientModelWithCheck(QObject *parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
}

int IngredientModelWithCheck::columnCount(const QModelIndex &parent) const
{
    // +1 for the checkbox column at index 0
    return QSqlTableModel::columnCount(parent) + 1;
}

bool IngredientModelWithCheck::select()
{
    bool ok = QSqlTableModel::select();
    // keep checkbox vector same size as rowCount
    m_checks = QVector<Qt::CheckState>(rowCount(), Qt::Unchecked);
    return ok;
}

QVariant IngredientModelWithCheck::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int r = index.row();
    int c = index.column();

    // Our virtual checkbox column
    if (c == 0) {
        if (role == Qt::CheckStateRole) {
            if (r >= 0 && r < m_checks.size())
                return m_checks[r];
            return Qt::Unchecked;
        }
        return QVariant(); // no text
    }

    // Shift real columns by -1
    QModelIndex sourceIndex = this->QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::data(sourceIndex, role);
}

bool IngredientModelWithCheck::setData(const QModelIndex &index,
                                       const QVariant &value,
                                       int role)
{
    if (!index.isValid())
        return false;

    int r = index.row();
    int c = index.column();

    // --- Checkbox column ---
    if (c == 0 && role == Qt::CheckStateRole) {

        qDebug() << "Checkbox toggled row=" << r;

        if (r < 0 || r >= rowCount())
            return false;

        // Update internal state
        Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
        m_checks[r] = newState;

        emit dataChanged(index, index, {Qt::CheckStateRole});

        // --- DB UPDATE ---
        int ingredientId =
            QSqlTableModel::data(QSqlTableModel::index(r, 1), Qt::DisplayRole).toInt();

        if (m_currentFoodId < 0)
            return true;

        QSqlQuery q(database());

        if (newState == Qt::Checked) {
            q.prepare("INSERT OR IGNORE INTO food_ingredients (food_id, ingredient_id) "
                      "VALUES (:f, :i)");
            q.bindValue(":f", m_currentFoodId);
            q.bindValue(":i", ingredientId);
            q.exec();
            qDebug() << "Inserted pivot row";
        } else {
            q.prepare("DELETE FROM food_ingredients WHERE food_id = :f AND ingredient_id = :i;");
            q.bindValue(":f", m_currentFoodId);
            q.bindValue(":i", ingredientId);
            q.exec();
            qDebug() << "Deleted pivot row";
        }

        return true;
    }

    // --- Other real SQL columns ---
    QModelIndex src = QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::setData(src, value, role);
}

Qt::ItemFlags IngredientModelWithCheck::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    int c = index.column();

    if (c == 0) {
        // Checkbox column: selectable + enabled + user-checkable
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }

    // Other columns: same as normal
    int r = index.row();
    QModelIndex sourceIndex = this->QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::flags(sourceIndex);
}

QVariant IngredientModelWithCheck::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return QString();   // empty header over checkboxes
        return QSqlTableModel::headerData(section - 1, orientation, role);
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

void IngredientModelWithCheck::setCheckedRows(const QSet<int> &ingredientIds)
{
    for (int i = 0; i < rowCount(); ++i)
    {
        int ingredientId = QSqlTableModel::data(
                               QSqlTableModel::index(i, 0), Qt::DisplayRole).toInt();

        m_checks[i] = ingredientIds.contains(ingredientId)
                          ? Qt::Checked
                          : Qt::Unchecked;
    }

    // Notify the view
    emit dataChanged(index(0,0), index(rowCount()-1, 0), {Qt::CheckStateRole});
}

void IngredientModelWithCheck::setCurrentFoodId(int id)
{
    m_currentFoodId = id;
}
