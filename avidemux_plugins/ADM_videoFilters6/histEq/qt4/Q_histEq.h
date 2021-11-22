#ifndef Q_histEq_h
#define Q_histEq_h
#include "ui_histEq.h"
#include "ADM_image.h"
#include "histEq.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyHistEq.h"

class Ui_histEqWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyHistEq *     myFly;
    ADM_QCanvas *      canvas;
    Ui_histEqDialog ui;

  public:
    Ui_histEqWindow(QWidget *parent, histEq *param,ADM_coreVideoFilter *in);
    ~Ui_histEqWindow();

  public slots:
    void gather(histEq *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_histEq_h
