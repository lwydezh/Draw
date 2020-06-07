#include "draw.h"
#include <random>

Draw::Draw()
{
    for(int i = 0; i <=  (int)Outstanding; i++)
    {
        mNumberMap.insert((Rank)(i), 0);
    }

    //    QString defaultStr = "赵 钱 孙 李 周 吴 郑 王 冯 陈 楮 卫 蒋 沈 韩 杨 朱 秦 尤 许 何 吕 施 张 孔 曹 严 华 金 魏 陶 姜 戚 谢 邹 喻 柏 水 窦 章 云 苏 潘 葛 奚 范 彭 郎 鲁 韦 昌 马 苗 凤 花 方 俞 任 袁 柳 酆 鲍 史 唐 费 廉 岑 薛 雷 贺 倪 汤 滕 殷 罗 毕 郝 邬 安 常 乐 于 时 傅 皮 卞 齐 康 伍 余 元 卜 顾 孟 平 黄 和 穆 萧 尹";
    QString defaultStr = "郭靖 黄蓉 乔峰 段誉 虚竹 王语嫣 阿珠 阿紫 杨过 小龙女 张无忌 赵敏 周芷若 小昭 令狐冲 任盈盈 韦小宝 陈家洛 香香公主 霍青桐 袁承志 夏青青 阿九 胡一刀 苗人凤  胡斐 袁紫衣 程灵素 苗若兰 狄云 水笙 戚芳 石破天 丁当 李文秀 周伯通";
    setNameList(toStringList(defaultStr));
}

void Draw::setNameList(const QStringList &list)
{
    if(mNameList != list)
    {
        mNameList = list;
        mIndexOfCurrent.clear();

        for(int index = 0; index < mNameList.size(); index++)
            mIndexOfCurrent.append(index);

        for(int i = First; i < Outstanding; i++)
            mIndexOfWinPeople[(Rank)i].clear();
    }
}

QStringList Draw::nameList() const
{
    return mNameList;
}

QString Draw::nameAt(const int index)
{
    QString str;
    if(index > -1 && index < mNameList.size())
    {
        str = mNameList.at(index);
    }

    return str;
}

void Draw::setNumberOfPeople(Draw::Rank rank, const int number)
{
    mNumberMap[rank] = qMax(0, number);
}

int Draw::numberOfPeople(Draw::Rank rank) const
{
    return mNumberMap[rank];
}

void Draw::setCurrentRank(Draw::Rank rank)
{
    mCurrentRank = rank;
}

Draw::Rank Draw::currentRank() const
{
    return mCurrentRank;
}

bool Draw::isRankComplete(Draw::Rank rank) const
{
    return (mIndexOfWinPeople[rank].size() == mNumberMap[rank]);
}

bool Draw::isCurrentLoopValid(QString *errorMsg) const
{
    QString msg;
    bool ret = true;
    if(mNameList.size() == 0)
    {
        msg = QString("当前参与人数为空。");
        ret = false;
    }
    if(mNameList.size() > 500)
    {
        msg = QString("抱歉，当前版本总人数暂不支持500以上。");
        ret = false;
    }
    else if(mIndexOfCurrent.size() == 0)
    {
        msg = QString("当前所有人数都已经获奖。");
        ret = false;
    }
    else if(mNumberMap[mCurrentRank] == 0)
    {
        msg = QString("本奖次设定人数为空。");
        ret = false;
    }
    else if(mNumberMap[mCurrentRank] == mIndexOfWinPeople[mCurrentRank].size())
    {
        msg = QString("本奖次已经抽取完毕。");
        ret = false;
    }
    else if(isNumberConflict())
    {
        msg = QString("所有奖次设定人数之和超出总人数。");
        ret = false;
    }
    //    else if(mNumberMap[mCurrentRank] >= 10)
    //    {
    //        msg = QString("抱歉，当前版本仅支持每个奖次人数不超过10人。");
    //        ret = false;
    //    }

    if(errorMsg != nullptr)
        *errorMsg = msg;

    return ret;
}

int Draw::indexOfCurrentLoopAboutToWin()
{
    Q_ASSERT(mIndexOfCurrent.size() != 0);

#if 1
    int i = getRandIndex(1, mIndexOfCurrent.size()) - 1;
#else
    int i = qrand() % mIndexOfCurrent.size();
#endif

    return mIndexOfCurrent.at(i);
}

int Draw::indexOfCurrentLoop()
{
#if 1
    int i = getRandIndex(1, mIndexOfCurrent.size()) - 1;
#else
    int i = qrand() % mIndexOfCurrent.size();
#endif

    int index = mIndexOfCurrent.at(i);
    mIndexOfCurrent.removeOne(index);
    mIndexOfWinPeople[mCurrentRank].append(index);

    return index;
}

QVector<int> Draw::getWinPeopleIndex(Draw::Rank rank)
{
    return mIndexOfWinPeople[rank];
}

void Draw::reset()
{
    for(int index = 0; index < mNameList.size(); index++)
        mIndexOfCurrent.append(index);

    for(int i = First; i <= (int)Outstanding; i++)
    {
        mNumberMap[(Rank)(i)] = 0;
        mIndexOfWinPeople[(Rank)(i)].clear();
    }
}

int Draw::getRandIndex(const int min, const int max)
{
    std::random_device sd;//生成random_device对象sd做种子
    std::minstd_rand linearRan(sd());//使用种子初始化linear_congruential_engine对象，为的是使用它来做我们下面随机分布的种子以输出多个随机分布.注意这里要使用()操作符，因为minst_rand()接受的是一个值（你用srand也是给出这样的一个值）

    std::uniform_int_distribution<int> dis1(min, max);
    return dis1(linearRan);
}

bool Draw::isNumberConflict() const
{
    int sum = 0;
    for(int i = First; i <= Outstanding; i++)
    {
        sum += mNumberMap[(Rank)i];
    }

    return sum > mNameList.size();
}
