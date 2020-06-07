#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <QComboBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QSize>
#include <qmath.h>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

#define INVALID_VALUE -1
#define OFFSET_X 45
#define OFFSET_Y 35
#define DISPLAY_W 450
#define DISPLAY_H 400

//#define TEST 1

const QString initLabelStyle("QLabel { background-color: transparent; border:none;}");
const QString selectedLabelStyle("QLabel { background-color: rgb(192, 0, 0); color: rgb(255, 215, 0); border-radius:2px; font: 16pt \"楷体\";}");
const QRect centerRect(210, 180, 120, 80);

void calculateRowColumn(const int total, int &row, int &column)
{
    row    = 0;
    column = 0;

    for(int i = 1; i <= total; i++)
    {
        if(total == i * i)
        {
            row = column = i;
            break;
        }

        if(total > i * i && total < (i + 1) * (i + 1))
        {
            column = i + 1;
            row = qCeil(total * 1.0 / column);
            break;
        }
    }
}

void setCheckState(QPushButton *btn, bool checked = true)
{
    btn->blockSignals(true);
    btn->setChecked(checked);
    btn->blockSignals(false);
}

void setComboboxCurrentIndex(QComboBox *box, const int index)
{
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(false);
}

void setSpinboxValue(QSpinBox *spinBox, const int value)
{
    spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(false);
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    pix.load(":/image/res/background-new.png");
    resize(pix.size());

    initSettings();
    initTimers();
    initLabels();
    updateResultPanel();
    ui->GoodLuckListPlainTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);

#ifndef TEST
    ui->StartTestBtn->setVisible(false);
    ui->StopTestBtn->setVisible(false);
    ui->TestCount->setVisible(false);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, pix);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton
            && p != QPointF(INVALID_VALUE, INVALID_VALUE))
    {
        move(this->pos() + event->globalPos() - p);
        p = event->globalPos();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        p = event->globalPos();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    p = QPoint(INVALID_VALUE, INVALID_VALUE);
}

void MainWindow::on_SetupBtn_clicked()
{
    if(ui->stackedWidget->currentIndex() == 0)
        return;

    updateWinPeopleInfo();
    ui->ExecBtn->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_RunBtn_clicked()
{
    if(ui->stackedWidget->currentIndex() == 1)
        return;

    drawObj.setNameList(toStringList(ui->NameListPlainTextEdit->toPlainText()));

    drawObj.setCurrentRank((Draw::Rank)ui->RankComboBox->currentIndex());
    drawObj.setNumberOfPeople(drawObj.currentRank(), ui->NumberSpinBox->value());

    initLabels();
    updateResultPanel();
    ui->ExecBtn->setVisible(true);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_ExecBtn_toggled(bool checked)
{
    if(checked)
    {
        QString msg;
        if(!drawObj.isCurrentLoopValid(&msg))
        {
            QMessageBox::warning(this, QString("注意"), msg, QMessageBox::Ok);
            setCheckState(ui->ExecBtn, false);
            return;
        }

        ui->ExecBtn->setText("停止");
        ui->SetupBtn->setEnabled(false);
        ui->RunBtn->setEnabled(false);
        processingTimer.start();
    }
    else
    {
        processingTimer.stop();
        int index = drawObj.indexOfCurrentLoop();
        if(pre)
            pre->setStyleSheet(initLabelStyle);
        mLabels.at(index)->setStyleSheet(selectedLabelStyle);
        playLabelsAnimation(index);
    }
}

void MainWindow::on_AddNameListBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择名单txt文件"),
                                                    "",
                                                    tr("文本文件 (*.txt)"));
    QFile file(fileName);
    if(!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString name = QString::fromStdString(file.readAll().toStdString());
    ui->NameListPlainTextEdit->setPlainText(name);
}

void MainWindow::on_RankComboBox_currentIndexChanged(int index)
{
    Draw::Rank rank = (Draw::Rank)(index);
    setSpinboxValue(ui->NumberSpinBox, drawObj.numberOfPeople(rank));

    updateWinPeopleInfo();
}

void MainWindow::on_MinimizedBtn_clicked()
{
    setWindowState(Qt::WindowMinimized);
}

void MainWindow::on_CloseBtn_clicked()
{
    saveResult();
    close();
}

void MainWindow::starTest()
{
    testCount = 0;
    mTestStop = false;
    test.clear();
    for(int i = 0; i < drawObj.nameList().size(); i++)
    {
        test.insert(i, 0);
    }

    testTimer.start();
}

void MainWindow::stopTest()
{
    mTestStop = true;
}

void MainWindow::initSettings()
{
    ui->NameListPlainTextEdit->setPlainText(toString(drawObj.nameList()));

    setComboboxCurrentIndex(ui->RankComboBox, (int)drawObj.currentRank());
    setSpinboxValue(ui->NumberSpinBox, drawObj.numberOfPeople(drawObj.currentRank()));

    updateWinPeopleInfo();
}

void MainWindow::initLabels()
{
    if(!mLabels.isEmpty())
    {
        qDeleteAll(mLabels);
        mLabels.clear();
    }
    pre = curr = nullptr;

    if(drawObj.nameList().size() == 0)
        return;

    int row = 0, column = 0;
    calculateRowColumn(drawObj.nameList().size(), row, column);
    int labelWidth = qFloor(DISPLAY_W / column);
    int labelHeight = qFloor(DISPLAY_H / row);

    QLabel *newLabel = nullptr;
    for(int i = 0; i < drawObj.nameList().size(); i++)
    {
        newLabel = new QLabel(ui->DisplayWidget);
        newLabel->setAlignment(Qt::AlignCenter);
        newLabel->setText(drawObj.nameAt(i));
        newLabel->setToolTip(newLabel->text());
        newLabel->setStyleSheet(initLabelStyle);
        newLabel->setFont(QFont("楷体", 12));

        int labelRow    = qFloor(i * 1.0 / column);
        int labelColumn = i - labelRow * column;

        newLabel->setGeometry(labelColumn * labelWidth + OFFSET_X,
                              labelRow * labelHeight + OFFSET_Y,
                              labelWidth,
                              labelHeight);
        if(newLabel->width() > centerRect.width() ||
                newLabel->height() > centerRect.height())
        {
            QPoint center = newLabel->geometry().center();
            int newW = qMin(newLabel->width(), centerRect.width());
            int newH =  qMin(newLabel->height(), centerRect.height());
            QPointF newPos = QPoint(center.x() - newW / 2.0, center.y() - newH / 2.0);
            newLabel->setGeometry(newPos.x(), newPos.y(), newW, newH);
        }

        mLabels.append(newLabel);
    }
}

void MainWindow::initTimers()
{
    processingTimer.setInterval(50);
    connect(&processingTimer, &QTimer::timeout,
            [=]()
    {
        int index = drawObj.indexOfCurrentLoopAboutToWin();
        curr = mLabels.at(index);
        if(pre)
            pre->setStyleSheet(initLabelStyle);
        curr->setStyleSheet(selectedLabelStyle);
        pre = curr;
    });

#ifdef TEST
    testTimer.setInterval(2);
    connect(&testTimer, &QTimer::timeout,
            [=]()
    {
        if(!mTestStop)
        {
            testCount++;
            test[drawObj.indexOfCurrentLoopAboutToWin()]++;
            ui->TestCount->setText(QString("%1").arg(testCount));
        }
        else
        {
            testTimer.stop();
            QJsonObject json;

            json["TEST-Date"]  = QDateTime::currentDateTime().toString("yyyyMMdd-hh:mm:ss");
            json["TEST-COUNT"] = testCount;

            for(int i = 0; i < drawObj.nameList().size(); i++)
            {
                json[drawObj.nameAt(i)] = test[i];
            }

            QJsonDocument doc(json);
            QString filePath = qApp->applicationDirPath() + "/testResult.txt";
            QFile file(filePath);
            if(file.open((QFile::Append)))
            {
                file.write(doc.toJson());
            }
        }
    });
#endif
}

void MainWindow::playLabelsAnimation(const int index)
{
    if(!animGroup)
    {
        animGroup = new QSequentialAnimationGroup(this);
        connect(animGroup, &QSequentialAnimationGroup::finished,
                [=]()
        {
            animGroup->clear();
            ui->ExecBtn->setText("开始");
            ui->ExecBtn->setEnabled(true);
            ui->SetupBtn->setEnabled(true);
            ui->RunBtn->setEnabled(true);
            updateResultPanel();
        });
    }
    ui->ExecBtn->setEnabled(false);

    mLabels.at(index)->raise();
    QPropertyAnimation *animation1 = new QPropertyAnimation(mLabels.at(index), "geometry");
    animation1->setDuration(1000);
    animation1->setStartValue(mLabels.at(index)->geometry());
    animation1->setEndValue(centerRect);

    QPropertyAnimation *animation2 = new QPropertyAnimation(mLabels.at(index), "geometry");
    animation2->setDuration(1000);
    animation2->setStartValue(centerRect);
    animation2->setEndValue(QRect(510,
                                  60 + 31 * (drawObj.getWinPeopleIndex(drawObj.currentRank()).size() - 1),
                                  120,
                                  31));

    animGroup->addPause(500);
    animGroup->addAnimation(animation1);
    animGroup->addPause(500);
    animGroup->addAnimation(animation2);

    animGroup->start();
}

void MainWindow::updateWinPeopleInfo()
{
    QString text("当前中奖名单：");
    QVector<int> winPeople = drawObj.getWinPeopleIndex(
                (Draw::Rank)ui->RankComboBox->currentIndex());
    if(winPeople.size() == 0 ||
            drawObj.numberOfPeople((Draw::Rank)ui->RankComboBox->currentIndex()) == 0)
    {
        text += QString("（空）");
    }
    else
    {
        for(int i = 0; i < winPeople.size(); i++)
        {
            text.append(drawObj.nameAt(winPeople.at(i)));
            if(i != winPeople.size() - 1)
                text.append(",");
        }
    }

    ui->GoodLuckListPlainTextEdit->setPlainText(text);
}

void MainWindow::updateResultPanel()
{
    QString rankInfo = QString("(%1/%2)")
            .arg(drawObj.getWinPeopleIndex(drawObj.currentRank()).size())
            .arg(drawObj.numberOfPeople(drawObj.currentRank()));

    ui->RankInfoLabel->setText(ui->RankComboBox->currentText() + rankInfo);

    QVector<int> winPeople;
    for(int i = Draw::First; i <= Draw::Outstanding; i++)
    {
        winPeople = drawObj.getWinPeopleIndex((Draw::Rank)(i));
        foreach (int index, winPeople) {
            mLabels.at(index)->setVisible(false);
            mLabels.at(index)->setStyleSheet(selectedLabelStyle);
        }
    }

    winPeople = drawObj.getWinPeopleIndex(drawObj.currentRank());
    for(int j = 0; j < winPeople.size(); j++)
    {
        mLabels.at(winPeople.at(j))->setVisible(true);
        mLabels.at(winPeople.at(j))->setGeometry(QRect(510,
                                                       60 + 31 * j,
                                                       120,
                                                       31));
    }
}

void MainWindow::saveResult()
{
    QJsonObject json;
    json["Title"] = ui->TitleLabel->text();
    json["Date"]  = QDateTime::currentDateTime().toString("yyyyMMdd-hh:mm:ss");

    QStringList list;
    int numberOfPeople = 0;
    QVector<int> indexOfPeople;
    QString text;
    for(int i = Draw::First; i <= Draw::Outstanding; i++)
    {
        text.clear();
        indexOfPeople = drawObj.getWinPeopleIndex((Draw::Rank)i);
        numberOfPeople = drawObj.numberOfPeople((Draw::Rank)i);
        text = QString("(%1/%2) ").arg(indexOfPeople.size()).arg(numberOfPeople);
        for(int j = 0; j < indexOfPeople.size(); j++)
        {
            text.append(drawObj.nameAt(indexOfPeople.at(j)));
            if(j != indexOfPeople.size() - 1)
                text.append(",");
        }

        list.append(text);
    }

    json["1ST"] = list.at(Draw::First);
    json["2ND"] = list.at(Draw::Second);
    json["3RD"] = list.at(Draw::Third);
    json["4TH"] = list.at(Draw::Forth);
    json["5TH"] = list.at(Draw::Fifth);
    json["6TH"] = list.at(Draw::Sixth);
    json["7Outstanding"] = list.at(Draw::Outstanding);

    QJsonDocument doc(json);
    QString filePath = qApp->applicationDirPath() + "/result.txt";
    QFile file(filePath);
    if(file.open((QFile::Append)))
    {
        file.write(doc.toJson());
    }
}

void MainWindow::on_StartTestBtn_clicked()
{
    starTest();
}

void MainWindow::on_StopTestBtn_clicked()
{
    stopTest();
}

void MainWindow::on_TitleLineEdit_textEdited(const QString &arg1)
{
    ui->TitleLabel->setText(arg1);
}
