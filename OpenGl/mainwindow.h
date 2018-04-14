#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector3D>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;

    bool inRotationUpdate, inScaleUpdate;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_ResetRotationButton_clicked(bool checked);
    void on_RotationDialX_valueChanged(int value);
    void on_RotationDialY_valueChanged(int value);
    void on_RotationDialZ_valueChanged(int value);

    void on_ResetScaleButton_clicked(bool checked);
    void on_ScaleSlider_valueChanged(int value);

    void on_PhongButton_toggled(bool checked);
    void on_NormalButton_toggled(bool checked);
    void on_GouraudButton_toggled(bool checked);

    void on_mainView_rotationChanged(QVector3D);
    void on_mainView_scaleChanged(float);

    void on_LockSun_toggled(bool);
    void on_LockEarth_toggled(bool);
    void on_LockMars_toggled(bool);
    void on_LockHorse_toggled(bool);
    void on_LockHorse2_toggled(bool);

signals:
    void rotationXChanged(int);
    void rotationYChanged(int);
    void rotationZChanged(int);

    void cameraLockChanged(const QString&);

};

#endif // MAINWINDOW_H
