#ifndef INGREDIENTMODELWITHCHECK_H
#define INGREDIENTMODELWITHCHECK_H

#include <QSqlTableModel>
#include <QVector>

class IngredientModelWithCheck : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit IngredientModelWithCheck(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());
    void setCheckedRows(const QSet<int> &ingredientIds);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool select() override;   // to resize checkbox storage when data is reloaded
    void setCurrentFoodId(int id);
private:
    QVector<Qt::CheckState> m_checks;
    int m_currentFoodId = -1;
};

#endif // INGREDIENTMODELWITHCHECK_H
