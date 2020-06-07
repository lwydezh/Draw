#ifndef DRAW_H
#define DRAW_H

#include <QStringList>
#include <QMap>
#include <QVector>
#include <QDebug>

inline QString toString(const QStringList& list)
{
    QString str;

    for(int i = 0; i < list.size(); i++)
    {
        str.append(list.at(i));
        if(i != list.size() - 1)
            str.append(" ");
    }

    return str;
}

inline QStringList toStringList(const QString &string)
{
    if(string.isEmpty())

        return QStringList();
    else
        return string.simplified().split(QChar(' '),
                                         QString::SkipEmptyParts);
}

class Draw
{
public:
    enum Rank
    {
        First = 0,
        Second,
        Third,
        Forth,
        Fifth,
        Sixth,
        Outstanding,
    };

    Draw();

    void                     setNameList(const QStringList &list);
    QStringList              nameList() const;
    QString                  nameAt(const int index);

    void                     setNumberOfPeople(Rank rank, const int number);
    int                      numberOfPeople(Rank rank) const;

    void                     setCurrentRank(Rank rank);
    Rank                     currentRank() const;
    bool                     isRankComplete(Rank rank) const;

    bool                     isCurrentLoopValid(QString *errorMsg = Q_NULLPTR) const;
    int                      indexOfCurrentLoopAboutToWin();
    int                      indexOfCurrentLoop();
    QVector<int>             getWinPeopleIndex(Rank rank);

    void                     reset();

private:
    int                      getRandIndex(const int min = 0, const int max = 100);
    bool                     isNumberConflict() const;

    QStringList              mNameList;
    QVector<int>             mIndexOfCurrent;
    QMap<Rank, int>          mNumberMap;
    QMap<Rank, QVector<int>> mIndexOfWinPeople;

    Rank                     mCurrentRank = First;
};

#endif // DRAW_H
