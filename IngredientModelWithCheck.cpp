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

    if (c == 0 && role == Qt::CheckStateRole) {
        if (r < 0 || r >= rowCount())
            return false;
        if (r >= m_checks.size())
            m_checks.resize(rowCount());
        m_checks[r] = static_cast<Qt::CheckState>(value.toInt());
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }

    // Real DB-backed columns
    QModelIndex sourceIndex = this->QSqlTableModel::index(r, c - 1);
    return QSqlTableModel::setData(sourceIndex, value, role);
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
