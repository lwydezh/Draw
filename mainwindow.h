#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include <QSequentialAnimationGroup>

#include "draw.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void on_SetupBtn_clicked();

    void on_RunBtn_clicked();

    void on_ExecBtn_toggled(bool checked);

    void on_AddNameListBtn_clicked();

    void on_RankComboBox_currentIndexChanged(int index);

    void on_MinimizedBtn_clicked();

    void on_CloseBtn_clicked();

    void on_StartTestBtn_clicked();

    void on_StopTestBtn_clicked();

    void on_TitleLineEdit_textEdited(const QString &arg1);

private:
    void starTest();
    void stopTest();

    void initSettings();
    void initLabels();
    void initTimers();
    void playLabelsAnimation(const int index);
    void updateWinPeopleInfo();
    void updateResultPanel();
    void saveResult();

    Ui::MainWindow *ui;
    QPixmap pix;
    QPoint  p;

    QVector<QLabel*> mLabels;
    QLabel           *pre  = nullptr;
    QLabel           *curr = nullptr;
    QTimer           processingTimer;

    QSequentialAnimationGroup *animGroup = nullptr;

    Draw             drawObj;

    //for test;
    QMap<int, int>   test;
    QTimer           testTimer;
    bool             mTestStop = false;
    long             testCount = 0;
};

#endif // MAINWINDOW_H
