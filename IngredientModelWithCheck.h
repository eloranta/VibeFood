#ifndef INGREDIENTMODELWITHCHECK_H
#define INGREDIENTMODELWITHCHECK_H

#include <QSqlTableModel>
#include <QVector>
#include <QSet>

class IngredientModelWithCheck : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit IngredientModelWithCheck(QObject *parent = nullptr,
                                      const QSqlDatabase &db = QSqlDatabase());

    // Column layout:
    // 0 = checkbox
    // 1 = ingredient_id (SQL)
    // 2 = ingredient name (SQL)
    // 3 = amount (virtual, editable only when checked)

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    bool select() override;

    // Called when user selects a food
    void setCurrentFoodId(int foodId);

    // Called when MainWindow loads pivot rows
    void setCheckedRows(const QSet<int> &ingredientIds,
                        const QMap<int, QString> &amounts);

private:
    QVector<Qt::CheckState> m_checks;     // row → checked/unchecked
    QVector<QString> m_amounts;           // row → amount
    int m_currentFoodId = -1;             // current selected food
};

#endif // INGREDIENTMODELWITHCHECK_H
