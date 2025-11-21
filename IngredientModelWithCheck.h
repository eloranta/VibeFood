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

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool select() override;

    // Needed so DB sync knows which food is selected
    void setCurrentFoodId(int id);

    // Called by MainWindow when food changes
    void setCheckedRows(const QSet<int> &ingredientIds);

private:
    QVector<Qt::CheckState> m_checks;    // row → checked/unchecked
    int m_currentFoodId = -1;            // selected food
};

#endif // INGREDIENTMODELWITHCHECK_H
