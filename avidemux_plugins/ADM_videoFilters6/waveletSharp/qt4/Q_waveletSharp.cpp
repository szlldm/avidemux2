/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

			    GUI for cropping including autocrop
			    +Revisted the Gtk2 way
			     +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
    copyright            : (C) 2002/2007 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPushButton>
#include "Q_waveletSharp.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidWaveletSharp.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_waveletSharpWindow::Ui_waveletSharpWindow(QWidget *parent, waveletSharp *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyWaveletSharp( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(waveletSharp));
        myFly->showOriginal = false;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Strength,100,2)
        SPINNER(Radius,100,2)

#define TOGGLER(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        TOGGLER(HQ)

        connect( ui.pushButtonPeek,SIGNAL(pressed()),this,SLOT(peekPressed()));
        connect( ui.pushButtonPeek,SIGNAL(released()),this,SLOT(peekReleased()));


        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_waveletSharpWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_waveletSharpWindow::gather(waveletSharp *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(waveletSharp));
}
Ui_waveletSharpWindow::~Ui_waveletSharpWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_waveletSharpWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_waveletSharpWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoWaveletSharp::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_waveletSharpWindow::peekPressed(void)
{
    myFly->showOriginal = true;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}
void Ui_waveletSharpWindow::peekReleased(void)
{
    myFly->showOriginal = false;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}

void Ui_waveletSharpWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_waveletSharpWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyWaveletSharp::upload(void)
{
    Ui_waveletSharpDialog *w=(Ui_waveletSharpDialog *)_cookie;
    MYSPIN(Strength)->setValue((int)round(param.strength*100.0));
    MYSPIN(Radius)->setValue((int)round(param.radius*100.0));
    MYCHECK(HQ)->setChecked(param.highq);
    return 1;
}
uint8_t flyWaveletSharp::download(void)
{
    Ui_waveletSharpDialog *w=(Ui_waveletSharpDialog *)_cookie;
    param.strength=((float)MYSPIN(Strength)->value()) / 100.0;
    param.radius=((float)MYSPIN(Radius)->value()) / 100.0;
    param.highq=MYCHECK(HQ)->isChecked();
    return 1;
}
void flyWaveletSharp::setTabOrder(void)
{
    Ui_waveletSharpDialog *w=(Ui_waveletSharpDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SPIN(Strength)
    PUSH_SPIN(Radius)
    PUSH_TOG(HQ)
    controls.push_back(w->pushButtonPeek);


    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);

    QWidget *first, *second;

    for(std::vector<QWidget *>::iterator tor = controls.begin(); tor != controls.end(); ++tor)
    {
        if(tor+1 == controls.end()) break;
        first = *tor;
        second = *(tor+1);
        _parent->setTabOrder(first,second);
        //ADM_info("Tab order: %p (%s) --> %p (%s)\n",first,first->objectName().toUtf8().constData(),second,second->objectName().toUtf8().constData());
    }
}
/**
    \fn     DIA_getCropParams
    \brief  Handle crop dialog
*/
uint8_t DIA_getWaveletSharp(waveletSharp *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_waveletSharpWindow dialog(qtLastRegisteredDialog(), param,in);

    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }

    qtUnregisterDialog(&dialog);

    return ret;
}
//____________________________________
// EOF


