#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <queue>
int cpu1,cpu2,idle1,idle2,cpuflag,cflag;
QTimer * timer,*timer1,*timer2;
double avgu;
std::vector<double> pa,ppa,ppa2;
Cpuwd *cpuwidget;
Netwd* netwidget;
int tot=0,tot1=0;
int pushid;
long long pren,nxtn,pren2,nxtn2;
double deltn,deltn2;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Sysmonitor");
    timer = new QTimer(this);timer1 = new QTimer(this),timer2=new QTimer(this);

    cpuwidget = new Cpuwd(ui->cpuwidget);
    netwidget = new Netwd(ui->netwidget);
    cpuwidget->setGeometry(10,10,900,450);//!
    netwidget->setGeometry(10,10,900,450);

    //current tab info
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(timer1, SIGNAL(timeout()), this, SLOT(cpucalc()));
    timer->start(500);
    timer1->start(5);
    timer2->start(100);
}




MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::update()
{
    QFile f("/proc/cpuinfo");//for read file;only support "/"
    if (!f.open(QIODevice::ReadOnly))
        return;
    //while(!f.atEnd())
    ///proc目录下，对这个文件调用size() 总是返回0,你还是能够从此类文件中读取到数据；这些数据是为了相应你的read() 函数而间接产生出来的。但是，在这种情况下，你不能使用atEnd() 来判断是否还有数据可读（因为atEnd() 对于大小为0的文件返回true）
    QString tmp;int pos;
    while(1)
    {
        tmp=f.readLine();
        if ( (pos= tmp.indexOf("model name"))!=-1)
        {
            //bug: sprintf(tmp.toLocal8Bit().data(),"model name\t:%s",modelname);
            QString q=tmp.right(41);//QString q=tmp.mid(pos+12,tmp.length()-12);
            ui->modelnamelabel->setText(q);
            break;
        }
    }

    while(1)
    {
        tmp=f.readLine();
        
        if(((pos = tmp.indexOf("cpu MHz"))!=-1))//0
        {
            QString q=tmp.mid(pos+11,tmp.length()-11);
            ui->freqlabel->setText(q+QString("MHz"));
            break;
        }
    }

    while(1)
    {
        tmp=f.readLine();
        if(((pos = tmp.indexOf("cache size"))!=-1))//0
        {
            QString q=tmp.mid(pos+13,tmp.length()-12);
            ui->cachesizelabel->setText(q);
            break;
        }
    }
    f.close();

    f.setFileName("/proc/version");
    if (!f.open(QIODevice::ReadOnly))
        return;
    tmp=f.readLine();
    ui->kernellabel->setText(tmp.left(38));
    ui->oslabel->setText(tmp.mid(58,27));
    f.close();

    f.setFileName("/proc/uptime");
    if (!f.open(QIODevice::ReadOnly))
        return;
    tmp=f.readLine();
    double n1;
    sscanf(tmp.toStdString().c_str(),"%lf",&n1);
    int rundays =(int)n1/86400;
    int runhour=((int)n1%86400)/3600;
    int runmin =((int)n1%3600)/60;
    int runsecond =(int)n1%60;
    ui->modelnamelabel_3->setText(QString::number(rundays)+"days|"+QString::number(runhour)+"h|"+QString::number(runmin)+"min|"+QString::number(runsecond)+"s");
    f.close();
    QProcess process1;
    QString program ="/bin/sh";
    process1.start(program, QStringList() << "../tm.sh");
    while (process1.waitForFinished());
    ui->modelnamelabel_2->setText(process1.readAllStandardOutput());
    process1.kill();

}
void MainWindow::cpucalc()
{
    int user,nice,syste,iowait,irq,softirq;
    QString tmp;
    if(cpuflag==0)
    {
        QFile f("/proc/stat");
        if (!f.open(QIODevice::ReadOnly))
            return;
        tmp=f.readLine();
        sscanf(tmp.mid(4,tmp.length()-4).toStdString().c_str(),"%d%d%d%d%d%d%d",&user,&nice,&syste,&idle1,&iowait,&irq,&softirq);
        cpu1=user+nice+syste+idle1+iowait+irq+softirq;
        f.close();
        cpuflag=1;
    }
    else if(cpuflag==1)
    {
        QFile f("/proc/stat");
        f.open(QIODevice::ReadOnly);
        tmp=f.readLine();
        sscanf(tmp.mid(4,tmp.length()-4).toStdString().c_str(),"%d%d%d%d%d%d%d",&user,&nice,&syste,&idle2,&iowait,&irq,&softirq);
        cpu2=user+nice+syste+idle2+iowait+irq+softirq;
        double utilization;
        if(idle1 && cpu2!=cpu1 ) utilization=1-(double)(idle2-idle1)/(cpu2-cpu1);
        //ui->cpulabel->setText(QString::number(utilization*100)+QString("%"));
        //cpuwidget->update();
        if(cflag<10)
        {
            cflag++;
            avgu+=(utilization<0?0:utilization);
        }
        if(cflag==10)
        {
            cflag=0;
            double lastavgu = avgu;
            avgu/=10;
            if(qIsNaN(avgu))
                avgu=0;
            ui->cpulabel->setText(QString::number(avgu*100));
            cpuwidget->update();

        }
        cpuflag=0;
        //pa.push(utilization);
    }

}
void MainWindow::memgraph()
{
    QString tmp;
    int tnum,t1,t2;
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly))
        return;
    tmp=f.readLine();
    sscanf(tmp.mid(9,tmp.length()-9).toStdString().c_str(),"%d",&tnum);
    ui->memlabel->setText(QString::number(tnum)+QString("B"));
    tmp=f.readLine();
    sscanf(tmp.mid(8,tmp.length()-8).toStdString().c_str(),"%d",&t1);
    ui->MemFreebar->setValue((double)t1/tnum*100);
    tmp=f.readLine();
    sscanf(tmp.mid(13,tmp.length()-13).toStdString().c_str(),"%d",&t2);
    ui->MemAvailblebar->setValue((double)t2/tnum*100);
    tmp=f.readLine();
    sscanf(tmp.mid(8,tmp.length()-8).toStdString().c_str(),"%d",&t1);
    tmp=f.readLine();
    sscanf(tmp.mid(7,tmp.length()-7).toStdString().c_str(),"%d",&t2);
    ui->cachedbar->setValue((double)t2/tnum*100);
    ui->bufbar->setValue((double)t1/tnum*100);
    f.close();
}


void MainWindow::netgraph()
{
    QString tmp;
    long long n1,n2,n3,n4,n5,n6,n7,n8;//! %lld
    QFile f("/proc/net/dev");
    if (!f.open(QIODevice::ReadOnly))// forget can cause segmentation fault
    return ;
    tmp=f.readLine();
    tmp=f.readLine();
    tmp=f.readLine();
    QString q=tmp.mid(8,tmp.length()-8);
    sscanf(q.toStdString().c_str(),"%lld%lld%*d%*d%*d%*d%*d%*d%lld%lld",&n1,&n2,&n3,&n4);
    tmp=f.readLine();
    tmp=f.readLine();
    q=tmp.mid(8,tmp.length()-8);
    sscanf(q.toStdString().c_str(),"%lld%lld%*d%*d%*d%*d%*d%*d%lld%lld",&n5,&n6,&n7,&n8);
    //
    ui->rcvlabel->setText(QString::number(n6));
    ui->rcvlabel_2->setText(QString::number(n5));
    ui->tranlabel->setText(QString::number(n8));
    ui->tranlabel_2->setText(QString::number(n7));
    nxtn = (n5),nxtn2=(n7);
    if(pren)
    {
        deltn = (nxtn-pren)/100;//B/ms=KB/s
        deltn2 = (nxtn2-pren2)/100;
    }
    pren = nxtn,pren2=nxtn2;
    ui->dlabel->setText(QString::number(deltn)+"KB/s");
    ui->ulabel->setText(QString::number(deltn2)+"KB/s");
    netwidget->update();

}

void MainWindow::processupdate()
{
    QFile f("/proc/stat");
    if (!f.open(QIODevice::ReadOnly))
        return ;
    QString tmp;int pos;
    while(1)
    {
        tmp=f.readLine();
        if ( (pos= tmp.indexOf("processes"))!=-1)
        {
            QString q=tmp.mid(10,tmp.length()-10);
            ui->pronumlabel->setText(q);
        }
        else if((pos= tmp.indexOf("procs_running"))!=-1)
        {
            QString q=tmp.right(3);
            ui->prorunlabel->setText(q);
        }
        else if((pos= tmp.indexOf("procs_blocked"))!=-1)
        {
            QString q=tmp.mid(13,tmp.length()-13);
            ui->problklabel->setText(q);
            break;
        }
    }
    f.close();

    //procsshower
    //QDir qd("/proc");
    //QStringList qsList = qd.entryList();
    QProcess process1,process2,process3,process4;

    process1.setStandardOutputProcess(&process2);
    process2.setStandardOutputProcess(&process3);
    process3.setStandardOutputProcess(&process4);
    QString program ="/bin/sh";
    process1.start(program, QStringList() << "ps -aux");
    process2.start(program, QStringList() << "grep -v USER");
    process3.start(program, QStringList() << "sort -rn -k +3");
    process4.start(program, QStringList() << "head -10");
    //process2.setProcessChannelMode(QProcess::ForwardedChannels);

    // To be fair: you only need to wait here for a bit with shutdown,
    // but I will still leave the rest here for a generic solution
    while (process4.waitForFinished());
    ui->procsshower->setText(process4.readAllStandardOutput());
    process1.kill();//QProcess: Destroyed while process ("sort") is still running.
    process2.kill();
    process3.kill();
    process4.kill();

    return;

}
void MainWindow::processupdate1()
{
    QFile f("/proc/stat");
    if (!f.open(QIODevice::ReadOnly))
        return ;
    QString tmp;int pos;
    while(1)
    {
        tmp=f.readLine();
        if ( (pos= tmp.indexOf("processes"))!=-1)
        {
            QString q=tmp.mid(10,tmp.length()-10);
            ui->pronumlabel->setText(q);
        }
        else if((pos= tmp.indexOf("procs_running"))!=-1)
        {
            QString q=tmp.right(3);
            ui->prorunlabel->setText(q);
        }
        else if((pos= tmp.indexOf("procs_blocked"))!=-1)
        {
            QString q=tmp.mid(13,tmp.length()-13);
            ui->problklabel->setText(q);
            break;
        }
    }
    f.close();
    QProcess process1;
    QString program ="/bin/sh";

    if(pushid==1)//cpu
    process1.start(program, QStringList() << "../my.sh");
    else if(pushid==2) process1.start(program, QStringList() << "../my1.sh");
    else process1.start(program, QStringList() << "../my0.sh");
    while (process1.waitForFinished());
    ui->procsshower->setText(process1.readAllStandardOutput());
    process1.kill();
    return;
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    disconnect(timer,0,0,0);
    disconnect(timer1,0,0,0);
    disconnect(timer2,0,0,0);
    //delete cpuwidget;
    if(index==0)//cpu
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        connect(timer1, SIGNAL(timeout()), this, SLOT(cpucalc()));

    }
    else if(index==1)//memory
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(memgraph()));
    }
    else if(index==2)
    {
        connect(timer2,SIGNAL(timeout()),this,SLOT(netgraph()));
    }
    else if(index==3)
    {
        connect(timer,SIGNAL(timeout()),this,SLOT(processupdate1()));
    }
    else if(index==4)
    {
        connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    }
}

Cpuwd::Cpuwd(QWidget *parent) :QWidget(parent)
{

}
Cpuwd::~Cpuwd()
{

}
Netwd::Netwd(QWidget *parent) :QWidget(parent)
{

}
Netwd::~Netwd()
{

}
#define PASIZE 200
#define DX (double)750/PASIZE
void Cpuwd::paintEvent(QPaintEvent*)
{
    QPainter painters(this);
    //
    painters.setPen(QPen(Qt::blue,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    painters.drawRect(0,0,750,400);
    painters.drawText(0,420,QString::number(PASIZE*0.1)+QString("s"));//20s
    painters.drawText(200,420,"15s");
    painters.drawText(400,420,"10s");
    painters.drawText(600,420,"5s");
    //painters.drawText(780,420,"0s");
    painters.drawText(760,20,"100%");
    painters.drawText(760,120,"75%");
    painters.drawText(760,220,"50%");
    painters.drawText(760,320,"25%");
    painters.drawText(760,410,"0");
    painters.setPen(QPen(Qt::red,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    if(pa.size()<PASIZE)
    {
        tot++;
    }
    else
    {
        pa.erase(pa.begin());
    }
    pa.push_back((1-avgu)*400);
    avgu=0;
    //printf("%lf\n",1-avgu);
    for(int i=0;i< pa.size()-1;i++)
    {
        painters.drawLine(750+DX*(-tot+i),pa[i],750+DX*(-tot+i+1),pa[i+1]);
    }
}
void Netwd::paintEvent(QPaintEvent*)
{
    QPainter painters(this);
    //
    painters.setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    painters.drawRect(0,0,750,400);
    painters.drawText(0,420,"20s");//20s
    painters.drawText(200,420,"15s");
    painters.drawText(400,420,"10s");
    painters.drawText(600,420,"5s");
    //painters.drawText(780,420,"0s");
    painters.drawText(760,20,"10M");
    painters.drawText(760,120,"7.5M");
    painters.drawText(760,220,"5M");
    painters.drawText(760,320,"2.5M");
    painters.drawText(760,410,"0");

    if(ppa.size()<PASIZE)
    {
        tot1++;
    }
    else
    {
        ppa.erase(ppa.begin());
        ppa2.erase(ppa2.begin());
    }
    ppa.push_back((10000-deltn)*(double)0.04);
    ppa2.push_back((10000-deltn2)*(double)0.04);

    //printf("%lf\n",1-avgu);
    for(int i=0;i< ppa.size()-1;i++)
    {
        painters.setPen(QPen(Qt::red,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));//down
        painters.drawLine(750+DX*(-tot1+i),ppa[i],750+DX*(-tot1+i+1),ppa[i+1]);
        painters.setPen(QPen(Qt::blue,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));//up
        painters.drawLine(750+DX*(-tot1+i),ppa2[i],750+DX*(-tot1+i+1),ppa2[i+1]);
    }
}


void MainWindow::on_pushButton_clicked()
{
    pushid=2;
}

void MainWindow::on_pushButton_2_clicked()
{
    pushid=1;
}

void MainWindow::on_pushButton_3_clicked()//search
{
    if(!ui->lineEdit->text().isEmpty())
    {
        QProcess process;

        process.start("ps "+ui->lineEdit->text());
        while (process.waitForFinished());
        QString q=process.readAll();

        ui->procsshower_2->setText(q.mid(35,q.size()-35));
        process.kill();
    }
}

void MainWindow::on_pushButton_4_clicked()//delete
{
    if(!ui->lineEdit->text().isEmpty())
    {
        QProcess process;

        process.start("kill "+ui->lineEdit->text());
        while (process.waitForFinished());
        QString q=process.readAll();

        ui->procsshower_2->setText("");
        process.kill();
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    system("shutdown -h now");
}
