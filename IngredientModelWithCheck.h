#ifndef INGREDIENTMODELWITHCHECK_H
#define INGREDIENTMODELWITHCHECK_H

#include <QSqlTableModel>
#include <QMap>

class IngredientModelWithCheck : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit IngredientModelWithCheck(QObject *parent = nullptr,
                                      const QSqlDatabase &db = QSqlDatabase());

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool select() override;

    void setCurrentFoodId(int foodId);
    void setCheckedRows(const QSet<int> &ingredientIds,
                        const QMap<int, QString> &amounts);

private:
    QMap<int, Qt::CheckState> m_checks;  // key = ingredient_id
    QMap<int, QString> m_amounts;        // key = ingredient_id

    int m_currentFoodId = -1;

    int ingredientIdForRow(int row) const;
};

#endif // INGREDIENTMODELWITHCHECK_H
